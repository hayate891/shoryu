#include <string>
#include "../pugixml/pugixml.hpp"
#include "../http/url.hpp"

namespace net
{
namespace upnp
{

struct port_mapping
{
	const std::string& internal_client;
	unsigned short internal_port;
	unsigned short external_port;
	std::string protocol;
	bool enabled;
	unsigned int lease_duration;
	std::string description;
};

// To get local IP - try to connect to control_url and then get local_endpoint()
// Internet gateway device is an UPNP device for port-forwarding
struct inet_gateway
{
	typedef net::http::url url;

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
	/*
	bool get_external_ip(std::string& ip)
	{
		using namespace http;
		soap_request soap_req = { control_url , service_type, "GetExternalIPAddress" };
		if(soap_response res = execute_request(soap_req))
			ip = res.vars["NewExternalIPAddress"];
		else
			return false;
		return true;
	}

	bool insert(const port_mapping& mapping)
	{
	}
	bool upsert(const port_mapping& mapping)
	{
		soap_request soap_req = { control_url , service_type, "AddPortMapping" };
		soap_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		soap_req.params["NewProtocol"] = std::toupper(mapping.protocol);
		soap_req.params["NewInternalPort"] = boost::lexical_cast<std::string>(mapping.internal_port);
		soap_req.params["NewInternalClient"] = mapping.internal_host;
		soap_req.params["NewEnabled"] = "1";
		soap_req.params["NewRemoteHost"] = "";
		soap_req.params["NewLeaseDuration"] = "0";
		soap_req.params["NewPortMappingDescription"] = "test";

		if(soap_response res = execute_request(soap_req))
			std::cout << "AddPortMapping: OK\n";
		else
			std::cout << "AddPortMapping: fault_code = " << res.fault_code <<
				", fault_string = " << res.fault_string << "\n";
	}
	bool update(const port_mapping& mapping)
	{
		return true;
	}
	bool select(const port_mapping& mapping)
	{
		soap_request soap_req = { control_url , service_type, "GetSpecificPortMappingEntry" };
		soap_req.params["NewRemoteHost"] = "";
		soap_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		soap_req.params["NewProtocol"] = std::toupper(mapping.protocol);

		soap_req.params["NewInternalPort"] = boost::lexical_cast<std::string>(mapping.internal_port);
		soap_req.params["NewInternalClient"] = mapping.internal_host;
		soap_req.params["NewEnabled"] = "1";
		soap_req.params["NewLeaseDuration"] = "0";
		soap_req.params["NewPortMappingDescription"] = "test";

		if(soap_response res = execute_request(soap_req))
			std::cout << "GetSpecificPortMappingEntry: NewInternalClient=" << res.vars["NewInternalClient"]
				<< ", NewPortMappingDescription=" <<  res.vars["NewPortMappingDescription"] << "\n";
		else
			std::cout << "GetSpecificPortMappingEntry: fault_code = " << res.fault_code <<
				", fault_string = " << res.fault_string << "\n";
	}

	bool remove(const port_mapping& mapping)
	{
		using namespace http;
		soap_request soap_req = { control_url , service_type, "DeletePortMapping" };
		soap_req.params["NewExternalPort"] = boost::lexical_cast<std::string>(mapping.external_port);
		soap_req.params["NewRemoteHost"] = "";
		soap_req.params["NewProtocol"] = std::toupper(mapping.protocol);

		if(soap_response res = execute_request(soap_req))
			std::cout << "DeletePortMapping: OK\n";
		else
			std::cout << "DeletePortMapping: fault_code = " << res.fault_code <<
				", fault_string = " << res.fault_string << "\n";
	}
	*/
};

}
}