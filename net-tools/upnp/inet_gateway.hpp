#include <string>
#include <boost/algorithm/string/case_conv.hpp>

#include "../pugixml/pugixml.hpp"
#include "../http/url.hpp"
#include "../http/soap_request.hpp"
#include "../http/soap_response.hpp"

namespace net
{
namespace upnp
{

struct port_mapping
{
	std::string& internal_client;
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
	typedef net::http::url url;
	typedef http::soap_request request_type;
	typedef http::soap_request::response_type response_type;
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
		request_type soap_req = { control_url , service_type, "GetExternalIPAddress" };
		if(response_type res = execute_request(soap_req))
			ip = res.vars["NewExternalIPAddress"];
		else
			return false;
		return true;
	}
	bool add(const port_mapping& mapping)
	{
		request_type soap_req = { control_url , service_type, "AddPortMapping" };
		soap_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		soap_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
		soap_req.params["NewInternalPort"] = boost::lexical_cast<std::string>(mapping.internal_port);
		soap_req.params["NewInternalClient"] = mapping.internal_client;
		soap_req.params["NewEnabled"] = boost::lexical_cast<std::string>(mapping.enabled ? 1 : 0);
		soap_req.params["NewRemoteHost"] = "";
		soap_req.params["NewLeaseDuration"] = mapping.lease_duration;
		soap_req.params["NewPortMappingDescription"] = mapping.description;

		if(response_type res = execute_request(soap_req))
			return true;
		else
			return false;
	}
	bool find(port_mapping& mapping)
	{
		request_type soap_req = { control_url , service_type, "GetSpecificPortMappingEntry" };
		soap_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		soap_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
		soap_req.params["NewRemoteHost"] = "";

		if(response_type res = execute_request(soap_req))
		{
			mapping.enabled = boost::lexical_cast<unsigned char>(res.vars["NewEnabled"]) != 0;
			mapping.description = res.vars["NewPortMappingDescription"];
			mapping.external_port = boost::lexical_cast<unsigned short>(res.vars["NewExternalPort"]);
			mapping.internal_client = res.vars["NewInternalClient"];
			mapping.internal_port = boost::lexical_cast<unsigned short>(res.vars["NewInternalClient"]);
			mapping.lease_duration = boost::lexical_cast<unsigned int>(res.vars["NewLeaseDuration"]);
			mapping.protocol = res.vars["NewProtocol"];
			return true;
		}
		else
			return false;
	}

	bool remove(const port_mapping& mapping)
	{
		request_type soap_req = { control_url , service_type, "DeletePortMapping" };
		soap_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		soap_req.params["NewProtocol"] = boost::to_upper_copy(mapping.protocol);
		soap_req.params["NewRemoteHost"] = "";

		if(response_type res = execute_request(soap_req))
			return true;
		else
			return false;
	}
};

}
}