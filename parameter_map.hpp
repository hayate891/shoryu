#pragma once
#include <boost/algorithm/string/predicate.hpp>
#include <boost/unordered_map.hpp>
#include <boost/xpressive/xpressive.hpp>
#include "urlencode.hpp"

namespace net
{

// TODO: move this to /detail
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

class parameter_map : public boost::unordered_map<std::string, std::string, ihash, iequal_to>
{
public:
	bool parse(const std::string& str)
	{
		if(!str.length())
		{
			clear();
			return true;
		}
		using namespace boost::xpressive;
		mark_tag _name(1);
		mark_tag _value(2);

		sregex pm_regex = (_name= +~(set='=', '?', '&')) >> '=' >> (_value= *~(set='=', '?', '&'));

		sregex_iterator cur( str.begin(), str.end(), pm_regex );
		sregex_iterator end;
		
		if(cur == end)
			return false;
		else
			clear();

		for( ; cur != end; ++cur )
		{
			const smatch &what = *cur;
			std::string n = what[_name];
			std::string v = what[_value];
			insert(std::make_pair(what[_name], what[_value]));
		}
		return true;
	}
};


std::ostream& operator<<(std::ostream& os, const parameter_map& pm)
{
	for(auto it = pm.begin(); it != pm.end();)
	{
		os << it->first << "=" << urlencode<GenericFilter>(it->second);
		if(++it != pm.end())
			os << "&";
	}
	return os;
}
std::istream& operator>>(std::istream& is, parameter_map& pm)
{
	std::string str;
	if(is >> str)
		pm.parse(str);

	return is;
}
}