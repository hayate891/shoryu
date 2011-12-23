#pragma once
#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>

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
		using namespace boost::xpressive;
		mark_tag _protocol(1);
		mark_tag _user(2);
		mark_tag _password(3);
		mark_tag _host(4);
		mark_tag _port(5);
		mark_tag _path(6);
		mark_tag _params(7);
		mark_tag _anchor(8);

		auto safe_chars = _w | '-' | '.' | '!' | '*' | '\'' | '(' | ')' | '%';
		auto params_chars = ~(set='#', '?');
		auto anchor_chars = _;
		auto path_chars = safe_chars | '/' | ':';
		auto label_chars = _w | '-';
		auto protocol_chars = _w;
		sregex url_regex = 
			bos >>
			!(
				(_protocol= +protocol_chars) >> ':' >> repeat<2>('/') >>
				!(
					(_user= +safe_chars) >> 
					!(':' >> (_password= *safe_chars) ) >> '@'
				) >>
				!(_host= +label_chars >> *('.' >> +label_chars) ) >>
				!(':' >> (_port= +_d))
			) >>
			!(_path= '/' >> *path_chars ) >> 
			!( '?' >> (_params= *params_chars) ) >>
			!( '#' >> (_anchor= *anchor_chars) ) >>
			eos;

		smatch m;
		if(boost::xpressive::regex_match(_url, m, url_regex))
		{
			protocol = m[_protocol];
			user = m[_user];
			password = m[_password];
			if(m[_host].length())
				host = m[_host];
			else
				host = "localhost";
			if(m[_port].length())
				port = boost::lexical_cast<unsigned short>(m[_port]);
			else
				port = 0;
			path = m[_path];
			//params = boost::lexical_cast<parameter_map>(m[_params]);
			anchor = m[_anchor];
		}
	}

	std::string protocol;
	std::string user;
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