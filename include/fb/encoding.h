#ifndef __ENCODING_H__
#define __ENCODING_H__

#ifndef _WIN32
#include <locale>
#include <iconv.h>
#include <codecvt>
#endif
#include <cstdint>
#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <iomanip>

namespace fb {

enum class PLATFORM
{
    WINDOWS,
    LINUX,
    BOTH
};

std::string  cp949(const std::string& utf8);
std::string  utf8(const std::string& cp949);
std::string  CP949(const std::string& utf8, PLATFORM p = PLATFORM::LINUX);
std::string  UTF8(const std::string& cp949, PLATFORM p = PLATFORM::LINUX);
std::wstring W(const std::string& m);
std::string  M(const std::wstring& w);
std::string  name_with(const std::string& name);
std::string  name_with(const std::string& name, const std::pair<std::string, std::string>& postfix);
bool         assert_korean(const std::string& str);
std::string  url_encode(const std::string& value);

} // namespace fb

#endif