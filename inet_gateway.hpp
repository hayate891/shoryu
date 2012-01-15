#pragma once
#include <string>
#include "pugixml/pugixml.hpp"
#include "url.hpp"

namespace net
{
struct port_mapping
{
	std::string internal_client;
	unsigned short internal_port;
	unsigned short external_port;
	std::string protocol;
	bool enabled;
	unsigned int lease_duration;
	std::string description;
};

// Internet gateway device is an UPNP device for port-forwarding
struct inet_gateway
{
	url control_url;
	std::string service_type;
	std::string model_name;
	
	bool parse_info(const url& base_url, const pugi::xml_document& xml_doc);
	bool parse_info(const url& base_url, const std::string& xml);
	bool get_internal_ip(std::string& ip) const;
	bool get_external_ip(std::string& ip) const;
	bool add(const port_mapping& mapping) const;
	bool find(port_mapping& mapping) const;
	bool remove(const port_mapping& mapping) const;
};

}