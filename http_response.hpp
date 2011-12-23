#pragma once
#include <string>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include "parameter_map.hpp"


namespace net
{
// TODO: remove 'body' property and rename to http_responce_header
struct http_response
{
	std::string http_ver;
	int status_code;
	std::string status_msg;
	parameter_map headers;
	std::string body;

	operator void const*() const { return status_code == 200 ? this : 0; }
};
// TODO: operator<<http_response
// TODO: http-authorization - add method require_authorization(const std::string& realm)
// TODO: https

std::istream& operator>>(std::istream& is, http_response& r)
{
	r.status_code = 0;
	std::string line;
	if(!std::getline(is, line))
	{
		is.setstate(std::ios::failbit);
		return is;
	}
	
	using namespace boost::xpressive;
	//^HTTP/(\\d\\.\\d) (\\d+) (.+)
	sregex http_status_regex = bos >> "HTTP/" >> (s1= _d >> '.' >> _d) >> _s >> (s2= +_d) >> _s >> (s3= +_) >> eos;
	smatch m;
	if(boost::xpressive::regex_match(line, m, http_status_regex))
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
			//^([^:]+):\\s*(.*)$
			sregex header_regex = bos >> (s1= +~(set= ':')) >> ':' >> *_s >> (s2= +_) >> eos;
			if(boost::xpressive::regex_match(line, m, header_regex))
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