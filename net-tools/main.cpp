#include <iostream>
#include <boost/asio.hpp>
#include "upnp/discover.hpp"
#include "upnp/inet_gateway.hpp"

int main()
{
	boost::asio::io_service ios;
	using namespace net::upnp;
	discover d(ios);
	d.discover_async<inet_gateway>([&d](const boost::system::error_code& ec, inet_gateway& device) -> bool{
		// отправлять ошибки только в том случае, если это точно upnp-устройство с поддержкой port-forwarding,
		// но на каком-то этапе произошла ошибка
		if(ec)
			std::cout << "error: code=" << ec.value() << ", message=" << ec.message() << std::endl;
		else
		{
			std::cout << "inet_gateway found: " << device.control_url << std::endl;
			return true;
		}
		return false;
	} );
	ios.run();
	return 0;
}