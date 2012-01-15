#pragma once
#include <iostream>
#include <string>
#include "parameter_map.hpp"

namespace net
{

struct upnp_response
{
	int error_code;
	std::string error_description;
	parameter_map vars;

	operator void const*() const { return error_code ? 0 : this; }
};

std::istream& operator>>(std::istream& is, upnp_response& r);

}