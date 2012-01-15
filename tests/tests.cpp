#include <boost/asio.hpp>
#include "../upnp_device_finder.hpp"
#include "../inet_gateway.hpp"
#include "../encoding.hpp"
#define BOOST_TEST_MODULE net
#include <boost/test/unit_test.hpp>

// TODO: write more tests already!
// TODO: make a specialization of boost::asio::iostream that can handle all protocols universally

BOOST_AUTO_TEST_SUITE( net )
	BOOST_AUTO_TEST_CASE( test_local_router )
	{
		using namespace net;

		url test1 = "ftp://user:pass@abc.de.ru/path?abc=cba&t34=43t#anchor!";
		url test2 = "192.168.0.1:7500";

		std::string value = net::urldecode(net::urlencode("àáûðvalg!+"));

		boost::asio::io_service ios;
		upnp_device_finder<inet_gateway> f(ios);
		f.start();
		f.async_find([&f](inet_gateway& device, const boost::system::error_code& ec) {
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
		} );
		ios.run();
		f.stop();
	}
BOOST_AUTO_TEST_SUITE_END()