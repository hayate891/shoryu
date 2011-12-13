#pragma once
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include "parameter_map.hpp"


namespace net
{
namespace http
{

struct http_response
{
	std::string http_ver;
	int status_code;
	std::string status_msg;
	parameter_map headers;
	std::string body;

	operator void const*() const { return status_code == 200 ? this : 0; }
};
std::istream& operator>>(std::istream& is, http_response& r)
{
	r.status_code = 0;
	std::string line;
	if(!std::getline(is, line))
	{
		is.setstate(std::ios::failbit);
		return is;
	}
	static const boost::regex http_status_regex("HTTP/(\\d\\.\\d) (\\d+) (.+)");
	boost::smatch m;
	if(boost::regex_match(line, m, http_status_regex, boost::match_extra))
	{
		r.http_ver = m[1];
		r.status_code = boost::lexical_cast<int>(m[2]);
		r.status_msg = m[3];
	}
	else
	{
		is.setstate(std::ios::failbit);
		return is;
	}
	r.headers.clear();
	while(std::getline(is, line))
	{
		boost::trim(line);
		if(line.length())
		{
			static const boost::regex http_status_regex("([^:]+):\\s*(.*)");
			if(boost::regex_match(line, m, http_status_regex, boost::match_extra))
				r.headers[m[1]] = m[2];
		}
		else
			break;
	}
	r.body.clear();
	auto header = r.headers.find("content-length");
	if(header != r.headers.end())
	{
		auto content_length = boost::lexical_cast<std::string::size_type>(header->second);
		if(content_length)
			std::copy_n(std::istreambuf_iterator<char>(is), content_length, std::back_inserter(r.body));
	}
	else
		std::copy(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), std::back_inserter(r.body));
	return is;
}

}
}