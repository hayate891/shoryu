#include "encoding.hpp"
#include <algorithm>
#include <boost/xpressive/xpressive.hpp>

namespace net
{
// TODO: implement punycode
// TODO: support wstrings and iterators

std::string urlencode(const std::string& str)
{
	std::ostringstream oss;
	for (auto it = str.begin(); it != str.end(); ++it) {
		if(std::isalnum(*it, std::locale::classic()))
			oss << *it;
		else
			oss << '%' << std::setw(2) << std::setfill('0') << std::uppercase << std::hex
				<< std::char_traits<char>().to_int_type(*it);
	}

	return oss.str();
}

// result is an UTF-8 string
std::string urldecode(const std::string& str)
{
	using namespace boost::xpressive;
	auto format = [](const smatch& what) -> std::string
	{
		std::stringstream ss;
		ss.str(what[1].str());
		int byte;
		if(ss >> std::hex >> byte && byte >= 0 && byte < 256)
		{
			std::string str;
			str.push_back((char)byte);
			return str;
		}
		else
			return what[1].str();
	};
	sregex decoding_regex = '%' >> (s1= repeat<2>(xdigit) );
	return regex_replace( str, decoding_regex, format);
}

// http://tools.ietf.org/html/rfc3492
// WHY IS IT SO DAMN COMPLICATED?
std::string punycode(const std::string& str)
{
	return str;
}

std::string punydecode(const std::string& str)
{
	return str;
}

}