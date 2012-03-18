#pragma once
#include <boost/array.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>

namespace boost
{
	template <typename T, std::size_t N>
	inline std::size_t hash_value(const boost::array<T,N>& arr) 
	{ 
		return boost::hash_range(arr.begin(), arr.end()); 
	}
	template <typename T, std::size_t N>
	inline std::size_t hash_value(const std::array<T,N>& arr) 
	{ 
		return boost::hash_range(arr.begin(), arr.end()); 
	}
	template<typename Protocol>
	inline std::size_t hash_value(const boost::asio::ip::basic_endpoint<Protocol>& ep)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, ep.port());
		if(ep.address().is_v4()) {
			boost::hash_combine(seed, ep.address().to_v4().to_bytes());
		}
		else if(ep.address().is_v6()) {
			boost::hash_combine(seed, ep.address().to_v6().to_bytes());
		}
		return seed;
	}

}