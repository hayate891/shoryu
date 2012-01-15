#pragma once
#include "parameter_map.hpp"

#include <boost/xpressive/xpressive.hpp>

#include "encoding.hpp"

namespace net
{
std::ostream& operator<<(std::ostream& os, const parameter_map& pm)
{
	for(auto it = pm.begin(); it != pm.end();)
	{
		os << it->first << "=" << urlencode(it->second);
		if(++it != pm.end())
			os << "&";
	}
	return os;
}
std::istream& operator>>(std::istream& is, parameter_map& pm)
{
	std::string str;
	if(is >> str)
	{
		if(!str.length())
			pm.clear();
		else
		{
			using namespace boost::xpressive;
			mark_tag _name(1);
			mark_tag _value(2);

			sregex pm_regex = (_name= +~(set='=', '?', '&')) >> '=' >> (_value= *~(set='=', '?', '&'));

			sregex_iterator cur( str.begin(), str.end(), pm_regex );
			sregex_iterator end;
		
			if(cur == end)
				is.setstate(std::ios::failbit);
			else
			{
				pm.clear();
				for( ; cur != end; ++cur )
				{
					const smatch &what = *cur;
					std::string n = what[_name];
					std::string v = what[_value];
					pm.insert(std::make_pair(what[_name], what[_value]));
				}
			}
		}
	}
	return is;
}
}