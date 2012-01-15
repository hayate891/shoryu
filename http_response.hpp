#pragma once
#include <iostream>
#include <string>
#include "parameter_map.hpp"


namespace net
{
// TODO: remove 'body' property and rename to http_responce_header
// TODO: rename 'headers' -> 'fields'
struct http_response
{
	std::string http_ver;
	int status_code;
	std::string status_msg;
	parameter_map headers;
	std::string body;

	operator void const*() const { return status_code == 200 ? this : 0; }
};
// TODO: operator<<http_response
// TODO: https responce

std::istream& operator>>(std::istream& is, http_response& r);

}