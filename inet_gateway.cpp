#pragma once
#include "inet_gateway.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "upnp_request.hpp"
#include "upnp_response.hpp"
#include "execute_request.hpp"

namespace net
{

bool inet_gateway::parse_info(const url& base_url, const pugi::xml_document& xml_doc)
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

bool inet_gateway::parse_info(const url& base_url, const std::string& xml)
{
	pugi::xml_document xml_doc;
	pugi::xml_parse_result result = xml_doc.load_buffer((char*)xml.data(), xml.length());
	if (result)
		return parse_info(base_url, xml_doc);
	else
		return false;
}

bool inet_gateway::get_internal_ip(std::string& ip) const
{
	typedef boost::asio::ip::tcp protocol_type;

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
bool inet_gateway::get_external_ip(std::string& ip) const
{
	upnp_request upnp_req = { control_url , service_type, "GetExternalIPAddress" };
	upnp_response res;
	if(execute_request(upnp_req, res) && res)
		ip = res.vars["NewExternalIPAddress"];
	else
		return false;
	return true;
}
bool inet_gateway::add(const port_mapping& mapping) const
{
	// TODO: check required fields

	upnp_request upnp_req = { control_url , service_type, "AddPortMapping" };
	upnp_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
	upnp_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
	upnp_req.params["NewInternalPort"] = boost::lexical_cast<std::string>(mapping.internal_port);
	upnp_req.params["NewInternalClient"] = mapping.internal_client;
	upnp_req.params["NewEnabled"] = mapping.enabled ? "1" : "0";
	upnp_req.params["NewRemoteHost"] = "";
	upnp_req.params["NewLeaseDuration"] = boost::lexical_cast<std::string>(mapping.lease_duration);
	upnp_req.params["NewPortMappingDescription"] = mapping.description;

	upnp_response res;
	if(execute_request(upnp_req, res) && res)
		return true;
	else
		return false;
}
bool inet_gateway::find(port_mapping& mapping) const
{
	// TODO: check required fields

	upnp_request upnp_req = { control_url , service_type, "GetSpecificPortMappingEntry" };
	upnp_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
	upnp_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
	upnp_req.params["NewRemoteHost"] = "";
		
	std::string str = boost::lexical_cast<std::string>(upnp_req);

	upnp_response res;
	if(execute_request(upnp_req, res) && res)
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

bool inet_gateway::remove(const port_mapping& mapping) const
{
	// TODO: check required fields

	upnp_request upnp_req = { control_url , service_type, "DeletePortMapping" };
	upnp_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
	upnp_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
	upnp_req.params["NewRemoteHost"] = "";

	upnp_response res;
	if(execute_request(upnp_req, res) && res)
		return true;
	else
		return false;
}

}