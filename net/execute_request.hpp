#pragma once
#include "url.hpp"

namespace net
{

template<typename RequestType>
typename RequestType::response_type execute_request(const RequestType& request)
{
	typedef typename RequestType::response_type response_type;
	typedef typename boost::asio::ip::tcp::iostream iostream;

	const url& url = request.url;
	std::string port = url.port ? boost::lexical_cast<std::string>(url.port) : url.protocol;
	iostream stream(url.host, port);
	if(!stream)
		throw std::exception();
	stream << request;

	response_type response;
	if(stream >> response)
		return response;
	else
		throw std::exception();
}

}