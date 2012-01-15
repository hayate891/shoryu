#pragma once

#include <string>

namespace net
{
// UTF-8 string expected  (as in Linux std::string)
std::string urlencode(const std::string& str);
// result is an UTF-8 string (as in Linux std::string)
std::string urldecode(const std::string& str);
std::string punycode(const std::string& str);
std::string punydecode(const std::string& str);
}
