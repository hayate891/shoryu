#pragma once
#include <string>
#include <istream>

#include "parameter_map.hpp"
#include "..\pugixml\pugixml.hpp"
#include "http_response.hpp"

namespace net
{
namespace http
{

struct soap_response
{
	std::string fault_code;
	std::string fault_string;
	parameter_map vars;

	operator void const*() const { return fault_code.empty() ? this : 0; }
};

std::istream& operator>>(std::istream& is, soap_response& sr)
{
	http_response httpr;
	if(is >> httpr)
	{
		if(httpr)
		{
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_buffer_inplace((char*)httpr.body.data(), httpr.body.length());
			if(result)
			{
				if(pugi::xpath_node fault_code = doc.select_single_node("//faultcode"))
				{
					sr.fault_code = fault_code.node().child_value();
					if(pugi::xpath_node fault_string = doc.select_single_node("//faultstring"))
						sr.fault_string = fault_string.node().child_value();
				}
				else
				{
					pugi::xml_node responce_node = doc.first_child().first_child().first_child();
					for (pugi::xml_node var = responce_node.first_child(); var; var = var.next_sibling())
						sr.vars[var.name()] = var.child_value();
				}
			}
			else
			{
				sr.fault_code = "InvalidSoap";
				sr.fault_string = result.description();
			}
		}
		else
		{
			sr.fault_code = boost::lexical_cast<std::string>(httpr.status_code);
			sr.fault_string = httpr.status_msg;
		}
	}
	return is;
}

}
}