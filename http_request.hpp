#pragma once
#include <iostream>

#include "parameter_map.hpp"
#include "url.hpp"

namespace net
{
// TODO: remove 'body' property and rename to http_request_header
// TODO: remove 'get' property, get params reside in url anyway
// TODO: rename 'headers' -> 'fields'

struct http_request
{
	url url;
	std::string method;
	parameter_map get;
	parameter_map post;
	parameter_map headers;
	std::string body;
};

// TODO: operator>> http_request
// TODO: http-authorization - add method authorization(const std::string& user, const std::string& pass)
// TODO: https request

std::ostream& operator<<(std::ostream& o, const http_request& r);

}