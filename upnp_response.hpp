#pragma once
#include <string>
#include <istream>

#include "parameter_map.hpp"
#include "pugixml\pugixml.hpp"
#include "http_response.hpp"

namespace net
{

struct upnp_response
{
	int error_code;
	std::string error_description;
	parameter_map vars;

	operator void const*() const { return error_code ? 0 : this; }
};

std::istream& operator>>(std::istream& is, upnp_response& r)
{
	http_response httpr;
	r.error_code = 0;
	if(is >> httpr)
	{
		if(httpr)
		{
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_buffer_inplace((char*)httpr.body.data(), httpr.body.length());
			if(!result)
			{
				r.error_code = -1;
				r.error_description = result.description();
			}
			else
			{
				pugi::xml_node responce_node = doc.first_child().first_child().first_child();
				for (pugi::xml_node var = responce_node.first_child(); var; var = var.next_sibling())
					r.vars[var.name()] = var.child_value();
			}
		}
		else
		{
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_buffer_inplace((char*)httpr.body.data(), httpr.body.length());
			if(result && httpr.body.length())
			{
				if(pugi::xpath_node fault_code = doc.select_single_node("//errorCode"))
				{
					r.error_code = boost::lexical_cast<int>(fault_code.node().child_value());
					if(pugi::xpath_node fault_string = doc.select_single_node("//errorDescription"))
						r.error_description = fault_string.node().child_value();
				}
				else
					r.error_code = -1;
			}
			else
			{
				r.error_code = boost::lexical_cast<int>(httpr.status_code);
				r.error_description = httpr.status_msg;
			}
		}
	}
	return is;
}

}