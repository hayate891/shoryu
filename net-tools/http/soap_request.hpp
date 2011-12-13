#pragma once
#include <ostream>

#include "soap_response.hpp"
#include "parameter_map.hpp"
#include "url.hpp"
#include "http_request.hpp"

namespace net
{
namespace http
{

struct soap_request
{
	typedef soap_response response_type;

	url url;
	std::string service_type;
	std::string action;
	parameter_map params;
};

std::ostream& operator<<(std::ostream& o, const soap_request& sr)
{
	http_request httpr = { "POST", sr.url };
	httpr.headers["Content-Type"] = "text/xml";
	httpr.headers["SOAPAction"] = '"' + sr.service_type +  '#' + sr.action + '"';

	//pugixml doesn't support namespaces so we need to create soap manually

	std::ostringstream oss;
	oss << "<?xml version=\"1.0\"?>\r\n"
		"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
		"\t<soap:Body>\r\n"
		"\t\t<service:" << sr.action << " xmlns:service=\"" << sr.service_type << "\">\r\n";
	for(auto it = sr.params.begin(); it != sr.params.end(); ++it)
		oss << "\t\t\t<" << it->first << ">" << it->second << "</" << it->first << ">\r\n";

	oss << "\t\t</service:" << sr.action << ">\r\n"
		"\t</soap:Body>\r\n"
		"</soap:Envelope>\r\n";

	httpr.body = oss.str();
	return o << httpr;
}

}
}