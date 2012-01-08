#include <boost/asio.hpp>
#include "discover.hpp"
#include "inet_gateway.hpp"
#define BOOST_TEST_MODULE net
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( net )
	BOOST_AUTO_TEST_CASE( test_local_router )
	{
		std::string value = net::urlencode<GenericFilter>("àáûðvalg!+");

		boost::asio::io_service ios;
		using namespace net;
		discover d(ios);
		d.discover_async<inet_gateway>([&d](const boost::system::error_code& ec, inet_gateway& device) -> bool {
			net::port_mapping mapping;
			mapping.protocol = "TCP";
			mapping.external_port = 60012;

			if(device.find(mapping))
				BOOST_REQUIRE(device.remove(mapping));

			mapping.internal_port = 60012;
			mapping.enabled = false;
			mapping.lease_duration = 0;
			mapping.description = "port forwarding test";

			std::string ip;
			BOOST_REQUIRE(device.get_internal_ip(ip));
			mapping.internal_client = ip;
			BOOST_REQUIRE(device.add(mapping));
			return true;
		} );
		ios.run();
	}
BOOST_AUTO_TEST_SUITE_END()