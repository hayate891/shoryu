#pragma once
#include <iostream>
#include <string>

#include "parameter_map.hpp"
#include "url.hpp"

namespace net
{
struct upnp_request
{
	url url;
	std::string service_type;
	std::string action;
	parameter_map params;
};

std::ostream& operator<<(std::ostream& o, const upnp_request& r);

}