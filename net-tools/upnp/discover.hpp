#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/thread/mutex.hpp>

#include "../pugixml/pugixml.hpp"
#include "../http/http_response.hpp"
#include "../http/http_request.hpp"
#include "../http/url.hpp"
#include "../execute_request.hpp"

namespace net
{
namespace upnp
{

namespace
{
	static const char discovery_msg[] = "M-SEARCH * HTTP/1.1\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"ST:upnp:rootdevice\r\n"
		"MAN:\"ssdp:discover\"\r\n"
		"MX:3\r\n\r\n";

}

class discover
{
public:
	typedef boost::asio::ip::udp::endpoint endpoint_type;
	typedef boost::asio::ip::address_v4 address_type;
	typedef boost::asio::ip::udp::socket socket_type;
	typedef boost::asio::ip::udp protocol;
	typedef boost::asio::streambuf streambuf_type;
	typedef boost::asio::streambuf::const_buffers_type recvbuf_type;

	static const size_t buffer_size = 1500;

	discover(boost::asio::io_service& io_service)
		: _socket(io_service, endpoint_type(protocol::v4(), 0))
	{
		_streambuf.commit(1);
		_streambuf.consume(1);
	}

	template<typename DeviceType>
	void discover_async(std::function<bool(const boost::system::error_code&, DeviceType&)> handler)
	{
		cancel();

		static const endpoint_type multicast_endpoint = endpoint_type(address_type::from_string("239.255.255.250"), 1900);
		
		_socket.async_send_to(boost::asio::buffer(discovery_msg, sizeof(discovery_msg)),
			multicast_endpoint, [](const boost::system::error_code&, size_t){});

		_socket.async_receive(boost::asio::buffer(_streambuf.prepare(buffer_size)),
				boost::bind(&discover::device_discovery<DeviceType>, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, handler));
	}
	void cancel()
	{
		boost::mutex::scoped_lock lock(_cancel_mutex);
		_socket.cancel();
	}
protected:
	template <typename DeviceType>
	void device_discovery(const boost::system::error_code& ec, std::size_t bytes_received,
		std::function<bool(const boost::system::error_code&, DeviceType&)> handler)
	{
		boost::mutex::scoped_lock lock(_cancel_mutex);
		if (bytes_received > 0 && !ec)
		{
			_streambuf.commit(bytes_received);
			std::istream is(&_streambuf);
			http::http_response response;
			if(is >> response)
			{
				auto it = response.headers.find("location");
				if(it != response.headers.end())
				{
					http::url base_url = boost::trim_copy(it->second);
					http::http_request req = {"GET", base_url };
					if(http::http_response res = execute_request(req))
					{
						pugi::xml_document xml_doc;
						pugi::xml_parse_result result = xml_doc.load_buffer_inplace((char*)res.body.data(), res.body.length());
						DeviceType device;
						if (result && device.parse_info(base_url, xml_doc) && handler(ec, device))
						{
							_streambuf.consume(bytes_received);
							return;
						}
					}
				}
			}
			_streambuf.consume(bytes_received);
		}
		_socket.async_receive(boost::asio::buffer(_streambuf.prepare(buffer_size)),
			boost::bind(&discover::device_discovery<DeviceType>, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, handler));
	}
	streambuf_type _streambuf;
	socket_type _socket;
	boost::mutex _cancel_mutex;
};

}
}