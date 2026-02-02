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
#include <string_view>
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

std::string  cp949(std::string_view utf8);
std::string  utf8(std::string_view cp949);
std::string  CP949(std::string_view utf8, PLATFORM p = PLATFORM::LINUX);
std::string  UTF8(std::string_view cp949, PLATFORM p = PLATFORM::LINUX);
std::wstring W(std::string_view m);
std::string  M(const std::wstring& w);
std::string  name_with(std::string_view name);
std::string  name_with(std::string_view name, const std::pair<std::string, std::string>& postfix);
bool         assert_korean(std::string_view str);
std::string  url_encode(std::string_view value);

} // namespace fb

#endif