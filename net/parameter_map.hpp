#pragma once
#include <boost/algorithm/string/predicate.hpp>
#include <boost/unordered_map.hpp>
#include "urlencode.hpp"

namespace net
{

//move this to /detail
namespace
{
struct iequal_to: std::binary_function<std::string, std::string, bool>
{
	bool operator()(const std::string& s1, const std::string& s2) const
	{
		return boost::algorithm::iequals(s1, s2, std::locale());
	}
};

struct ihash: std::unary_function<std::string, std::size_t>
{
	std::size_t operator()(const std::string& s) const
	{
		std::size_t seed = 0;
		std::locale locale;

		for(auto it = s.begin(); it != s.end(); ++it)
			boost::hash_combine(seed, std::toupper(*it, locale));

		return seed;
	}
};
}

class parameter_map : public boost::unordered_map<std::string, std::string, ihash, iequal_to>{};

//TODO: operator>> param_map

std::ostream& operator<<(std::ostream& o, const parameter_map& pm)
{
	for(auto it = pm.begin(); it != pm.end();)
	{
		o << it->first << "=" << urlencode(it->second);
		if(++it != pm.end())
			o << "&";
	}
	return o;
}

}