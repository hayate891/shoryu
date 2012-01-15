#pragma once
#include <iostream>
#include <string>

#include "parameter_map.hpp"

namespace net
{

class invalid_url : public std::exception
{
public:
	invalid_url() throw();
};

struct url
{
	url();
	url(const char* ptr);
	url(const std::string& _url);

	// TODO: maybe get rid of this method and just do it in the constructor?
	bool parse(const std::string& _url);

	std::string protocol;
	std::string user;
	std::string password;
	std::string host;
	unsigned short port;
	std::string path;
	parameter_map params;
	std::string anchor;
};

std::ostream& operator<<(std::ostream& os, const url& url);

std::istream& operator>>(std::istream& is, url& url_);

}