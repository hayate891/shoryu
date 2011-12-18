#include <iostream>
#include <boost/asio.hpp>
#include "discover.hpp"
#include "inet_gateway.hpp"

int main()
{
	boost::asio::io_service ios;
	using namespace net;
	discover d(ios);
	d.discover_async<inet_gateway>([&d](const boost::system::error_code& ec, inet_gateway& device) -> bool {
		net::port_mapping mapping;
		mapping.protocol = "TCP";
		mapping.external_port = 60012;

		if(device.find(mapping))
			device.remove(mapping);

		mapping.internal_port = 60012;
		mapping.enabled = false;
		mapping.lease_duration = 0;
		mapping.description = "port forwarding test";

		std::string ip;
		if(device.get_internal_ip(ip))
			mapping.internal_client = ip;
		else
			return false;
		if(!device.add(mapping))
			return false;
		return true;
	} );
	ios.run();
	return 0;
}