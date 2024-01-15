#ifndef __ENCODING_H__
#define __ENCODING_H__

#ifdef _WIN32
#include <Windows.h>
#else
#include <locale.h>
#include <iconv.h>
#include <codecvt>
#endif
#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <cstring>

enum class PLATFORM
{
    Windows,
    Linux,
    Both
};

std::string                 cp949(const std::string& utf8);
std::string                 utf8(const std::string& cp949);
std::string                 CP949(const std::string& utf8, PLATFORM p = PLATFORM::Linux);
std::string                 UTF8(const std::string& cp949, PLATFORM p = PLATFORM::Linux);

std::wstring                W(const std::string& m);
std::string                 M(const std::wstring& w);

std::string                 fstring_c(const std::string& fmt, va_list* args);

#endif