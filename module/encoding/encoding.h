#ifndef	__ENCODING_H__
#define	__ENCODING_H__

#ifdef _WIN32
#include <Windows.h>
#else
#include <locale.h>
#include <iconv.h>
#endif
#include <wchar.h>
#include <stdlib.h>
#include <string>
#include <cstring>

std::string                 cp949(const std::string& utf8);
std::string                 utf8(const std::string& cp949);
std::string                 CP949(const std::string& utf8);
std::string                 UTF8(const std::string& cp949);
std::wstring				wcs(const std::string& mbs);

#endif