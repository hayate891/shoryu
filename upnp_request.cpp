#pragma once
#include "upnp_request.hpp"

#include <ostream>
#include <sstream>

#include "http_request.hpp"

namespace net
{
std::ostream& operator<<(std::ostream& o, const upnp_request& r)
{
	http_request httpr = { r.url, "POST" };
	httpr.headers["Content-Type"] = "text/xml";
	httpr.headers["SOAPAction"] = '"' + r.service_type +  '#' + r.action + '"';

	//pugixml doesn't support namespaces so we need to create soap manually

	std::ostringstream oss;
	oss << "<?xml version=\"1.0\"?>\r\n"
		"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
		"\t<soap:Body>\r\n"
		"\t\t<service:" << r.action << " xmlns:service=\"" << r.service_type << "\">\r\n";
	for(auto it = r.params.begin(); it != r.params.end(); ++it)
		oss << "\t\t\t<" << it->first << ">" << it->second << "</" << it->first << ">\r\n";

	oss << "\t\t</service:" << r.action << ">\r\n"
		"\t</soap:Body>\r\n"
		"</soap:Envelope>\r\n";

	httpr.body = oss.str();
	return o << httpr;
}

}