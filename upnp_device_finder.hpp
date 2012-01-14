#pragma once
#include <string>

#include <boost/asio.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>

#include "pugixml/pugixml.hpp"
#include "http_response.hpp"
#include "http_request.hpp"
#include "url.hpp"
#include "execute_request.hpp"

namespace net
{

namespace
{
	static const char discovery_msg[] = "M-SEARCH * HTTP/1.1\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"ST:upnp:rootdevice\r\n"
		"MAN:\"ssdp:discover\"\r\n"
		"MX:3\r\n\r\n";
}

template<class DeviceType>
class upnp_device_finder
{
	typedef boost::asio::ip::udp::socket socket_type;
	typedef boost::asio::ip::udp protocol;
	typedef boost::asio::streambuf streambuf_type;
	typedef boost::asio::ip::address_v4 address_type;
	typedef protocol::endpoint endpoint_type;

	typedef upnp_device_finder<DeviceType> this_type;
public:
	upnp_device_finder(boost::asio::io_service& io_service) : _socket(io_service) {}

	// Callback should never throw, otherwise memory leaks will occur
	template<typename CallbackType>
	void async_find(CallbackType& callback)
	{
		static const size_t buffer_size = 1500;
		_socket.async_receive(boost::asio::buffer(_streambuf.prepare(buffer_size)),
			boost::bind(&this_type::recv_handler<CallbackType>, this,
			boost::asio::placeholders::bytes_transferred, callback, boost::asio::placeholders::error));
	}
	void start(boost::system::error_code& ec)
	{
		if(!is_started()) init_socket(ec);
		if(ec) return;
		static const endpoint_type multicast_endpoint = endpoint_type(address_type::from_string("239.255.255.250"), 1900);
		_socket.send_to(boost::asio::buffer(discovery_msg, sizeof(discovery_msg)), multicast_endpoint, 0, ec);
	}
	void start()
	{
		boost::system::error_code ec;
		start(ec);
		boost::asio::detail::throw_error(ec, "start");
	}
	bool is_started() const
	{
		return _socket.is_open();
	}
	void stop(boost::system::error_code& ec)
	{
		if(_socket.is_open())
			_socket.close(ec);
	}
	void stop()
	{
		boost::system::error_code ec;
		stop(ec);
		boost::asio::detail::throw_error(ec, "stop");
	}
protected:
	void init_socket(boost::system::error_code& ec)
	{
		_socket.open(protocol::v4(), ec);
		if(ec) return;
		_socket.bind(endpoint_type(protocol::v4(), 0), ec);
	}

	template<typename CallbackType>
	void recv_handler(size_t bytes_received, CallbackType& callback, const boost::system::error_code& ec)
	{
		DeviceType device;
		if(ec)
		{
			callback(device, ec);
			return;
		}
		if (bytes_received > 0)
		{
			_streambuf.commit(bytes_received);
			std::istream is(&_streambuf);
			http_response response;
			if(is >> response)
			{
				auto it = response.headers.find("location");
				if(it != response.headers.end())
				{
					url base_url = boost::trim_copy(it->second);
					http_request req = { base_url, "GET" };
					http_response res;
					if(execute_request(req, res) && res)
					{
						pugi::xml_document xml_doc;
						pugi::xml_parse_result result = xml_doc.load_buffer_inplace((char*)res.body.data(), res.body.length());
						if (result && device.parse_info(base_url, xml_doc))
							callback(device, ec);
						else
							async_find(callback);
					}
				}
			}
			_streambuf.consume(bytes_received);
		}
	}

	streambuf_type _streambuf;
	socket_type _socket;
};

}