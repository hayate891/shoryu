#pragma once
#include <ostream>
#include <boost/lexical_cast.hpp>

#include "parameter_map.hpp"

namespace net
{

struct url
{
	url() : protocol("http"), path("/"), port(0) {}
	url(const char* ptr) : protocol("http"), path("/"), port(0)
	{
		parse(std::string(ptr));
	}
	url(const std::string& _url) : protocol("http"), path("/"), port(0) 
	{
		parse(_url);
	}

	void parse(const std::string& _url)
	{
		// TODO: refactoring, parse login, password, host etc
		// cannot parse "http://192.168.0.2:80" (empty path)
		static const boost::regex with_port("([a-z]+)://([^/]+):(\\d+)(/.*)");
		static const boost::regex without_port("([a-z]+)://([^/]+)(/.*)");
		static const boost::regex relative_path("/.*");
		boost::smatch m;
		if(boost::regex_match(_url, m, with_port, boost::match_extra))
		{
			protocol = m[1];
			host = m[2];
			port = boost::lexical_cast<unsigned short>(m[3]);
			path = m[4];
			return;
		}
		if(boost::regex_match(_url, m, without_port, boost::match_extra))
		{
			protocol = m[1];
			host = m[2];
			path = m[3];
			return;
		}
		path = _url;
	}

	std::string protocol;
	std::string login;
	std::string password;
	std::string host;
	unsigned short port;
	std::string path;
	parameter_map params;
	std::string anchor;
};

std::ostream& operator<<(std::ostream& o, const url& u)
{
	if(u.host.length())
	{
		o << u.protocol << "://" << u.host;
		if(u.port != 0)
			o << ":" << u.port;
	}
	o << u.path;
	return o;
}

}