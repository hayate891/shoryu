#pragma once
#include "url.hpp"

#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>

namespace net
{

invalid_url::invalid_url() throw() : std::exception("invalid url", 1){}

url::url() : protocol("http"), path("/"), port(0) {}
url::url(const char* ptr) : protocol("http"), path("/"), port(0)
{
	if(!parse(std::string(ptr)))
		throw invalid_url();
}
url::url(const std::string& _url) : protocol("http"), path("/"), port(0) 
{
	if(!parse(_url))
		throw invalid_url();
}
bool url::parse(const std::string& _url)
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
			!(
				(_protocol= +protocol_chars) >> ':' >> repeat<2>('/')
			) >>
			!(
				(_user= +safe_chars) >> 
				!(':' >> (_password= *safe_chars) ) >> '@'
			) >>
			!(
				(_host= +label_chars >> *('.' >> +label_chars)) >>
				!(':' >> (_port= +_d))
			)
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
		if(m[_params].length())
			params = boost::lexical_cast<parameter_map>(m[_params]);
		else
			params.clear();
		anchor = m[_anchor];
	}
	else
		return false;
	return true;
}

std::ostream& operator<<(std::ostream& os, const url& url)
{
	if(url.host.length())
	{
		if(url.protocol.length())
			os << url.protocol << "://";
		os << url.host;
		if(url.port != 0)
			os << ":" << url.port;
	}
	os << url.path;
	return os;
}

std::istream& operator>>(std::istream& is, url& url_)
{
	std::string str;
	if(is >> str)
		url_ = url(str);
	return is;
}

}