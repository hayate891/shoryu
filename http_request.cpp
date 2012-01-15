#pragma once
#include "http_request.hpp"
#include <boost/lexical_cast.hpp>

namespace net
{
std::ostream& operator<<(std::ostream& o, const http_request& r)
{
	o << r.method << " " << r.url.path;
	if(r.method == "GET" && r.get.size())
		o << "?" << r.get;

	o << " HTTP/1.1\r\n";
	o << "Host: " << r.url.host;
	if(r.url.port != 80)
		o << ":" << r.url.port;
	o << "\r\n";
	for(auto it = r.headers.begin(); it != r.headers.end(); ++it)
		o << it->first << ": " << it->second << "\r\n";
	std::string body = r.body;
	if( r.method == "POST" && !body.length() )
		body = boost::lexical_cast<std::string>(r.post);
	o << "Content-Length: " << body.size() << "\r\n\r\n" << body;
	return o;
}

}