#pragma once
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "url.hpp"

namespace net
{
// TODO: write accept<RequestType>() method - useful for servers (using iostream+acceptor)
template<typename RequestType, typename ResponseType>
bool execute_request(const RequestType& request, ResponseType& response)
{
	typedef typename boost::asio::ip::tcp::iostream iostream;

	const url& url = request.url;
	std::string port = url.port ? boost::lexical_cast<std::string>(url.port) : url.protocol;
	iostream stream(url.host, port);
	
	return stream && (stream << request) && (stream >> response);
}

}