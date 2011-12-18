#pragma once

#include <string>
#include <boost/algorithm/string/case_conv.hpp>

#include "pugixml/pugixml.hpp"
#include "url.hpp"
#include "upnp_request.hpp"
#include "upnp_response.hpp"

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
	typedef url url;
	typedef upnp_request request_type;
	typedef upnp_request::response_type response_type;
	typedef boost::asio::ip::tcp protocol_type;

	url control_url;
	std::string service_type;
	std::string model_name;
	
	bool parse_info(const url& base_url, const pugi::xml_document& xml_doc)
	{
		if(pugi::xpath_node device = xml_doc.select_single_node("//device/deviceType[contains(.,'InternetGatewayDevice')]/.."))
			model_name = device.node().child_value("modelName");

		pugi::xpath_node service;
		if((service = xml_doc.select_single_node("//service/serviceType[contains(.,'WANIPConnection')]/..")) ||
			(service = xml_doc.select_single_node("//service/serviceType[contains(.,'WANPPPConnection')]/..")))
		{
			control_url = service.node().child_value("controlURL");
			if(!control_url.host.length())
			{
				control_url.host = base_url.host;
				control_url.port = base_url.port;
				control_url.protocol = base_url.protocol;
			}
			service_type = service.node().child_value("serviceType");
		}
		else
			return false;
		return true;
	}
	bool parse_info(const url& base_url, const std::string& xml)
	{
		pugi::xml_document xml_doc;
		pugi::xml_parse_result result = xml_doc.load_buffer((char*)xml.data(), xml.length());
		if (result)
			return parse_info(base_url, xml_doc);
		else
			return false;
	}

	bool get_internal_ip(std::string& ip)
	{
		boost::asio::io_service ios;
		protocol_type::resolver resolver(ios);
		protocol_type::resolver::query query(protocol_type::v4(), control_url.host,
			boost::lexical_cast<std::string>(control_url.port));
		protocol_type::resolver::iterator iterator = resolver.resolve(query);
		if(iterator != protocol_type::resolver::iterator())
		{
			protocol_type::socket sock(ios, protocol_type::endpoint(protocol_type::v4(), 0));
			sock.connect(*iterator);
			ip = sock.local_endpoint().address().to_string();
			return true;
		}
		return false;
	}
	bool get_external_ip(std::string& ip)
	{
		request_type upnp_req = { control_url , service_type, "GetExternalIPAddress" };
		if(response_type res = execute_request(upnp_req))
			ip = res.vars["NewExternalIPAddress"];
		else
			return false;
		return true;
	}
	bool add(const port_mapping& mapping)
	{
		request_type upnp_req = { control_url , service_type, "AddPortMapping" };
		upnp_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		upnp_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
		upnp_req.params["NewInternalPort"] = boost::lexical_cast<std::string>(mapping.internal_port);
		upnp_req.params["NewInternalClient"] = mapping.internal_client;
		upnp_req.params["NewEnabled"] = mapping.enabled ? "1" : "0";
		upnp_req.params["NewRemoteHost"] = "";
		upnp_req.params["NewLeaseDuration"] = boost::lexical_cast<std::string>(mapping.lease_duration);
		upnp_req.params["NewPortMappingDescription"] = mapping.description;

		if(response_type res = execute_request(upnp_req))
			return true;
		else
			return false;
	}
	bool find(port_mapping& mapping)
	{
		request_type upnp_req = { control_url , service_type, "GetSpecificPortMappingEntry" };
		upnp_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		upnp_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
		upnp_req.params["NewRemoteHost"] = "";
		
		std::string str = boost::lexical_cast<std::string>(upnp_req);

		if(response_type res = execute_request(upnp_req))
		{
			mapping.enabled = boost::lexical_cast<unsigned char>(res.vars["NewEnabled"]) != 0;
			mapping.description = res.vars["NewPortMappingDescription"];
			mapping.internal_client = res.vars["NewInternalClient"];
			mapping.internal_port = boost::lexical_cast<unsigned short>(res.vars["NewInternalPort"]);
			mapping.lease_duration = boost::lexical_cast<unsigned int>(res.vars["NewLeaseDuration"]);
			return true;
		}
		else
			return false;
	}

	bool remove(const port_mapping& mapping)
	{
		request_type upnp_req = { control_url , service_type, "DeletePortMapping" };
		upnp_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		upnp_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
		upnp_req.params["NewRemoteHost"] = "";

		if(response_type res = execute_request(upnp_req))
			return true;
		else
			return false;
	}
};

}