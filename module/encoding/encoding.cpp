#include "encoding.h"

std::string utf8(const std::string& utf8)
{
#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), nullptr, 0) + 1;
    auto wide = new wchar_t[wide_size];
    memset(wide, 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), wide, wide_size);

    auto mbs_size = WideCharToMultiByte(CP_ACP, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    auto mbs = new char[mbs_size];
    memset(mbs, 0, mbs_size);
    WideCharToMultiByte(CP_ACP, 0, wide, -1, mbs, mbs_size, nullptr, nullptr); 

    auto ansi = std::string(mbs);
    delete[] mbs;
    delete[] wide;
    return ansi;
#else
    return utf8;
#endif
}

std::wstring wcs(const std::string& mbs)
{
#ifdef _WIN32
    auto size = (uint8_t)MultiByteToWideChar(CP_ACP, 0, mbs.c_str(), mbs.size(), nullptr, 0) + 1;
    auto buffer = new wchar_t[size];
    memset(buffer, 0, sizeof(wchar_t) * size);
    MultiByteToWideChar(CP_ACP, 0, mbs.c_str(), mbs.size(), buffer, sizeof(wchar_t) * size);
#else
    setlocale(LC_ALL, "Korean");
    auto size = mbs.length() + 1;
    auto buffer = new wchar_t[size];
    mbstowcs(buffer, mbs.c_str(), sizeof(wchar_t) * mbs.length());
#endif

    std::wstring wide(buffer);
    delete[] buffer;
    
    return wide;
}