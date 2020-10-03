#include "encoding.h"

std::string cp949(const std::string& utf8)
{
    if(utf8.empty())
        return std::string();

#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), nullptr, 0) + 1;
    auto wide = new wchar_t[wide_size];
    memset(wide, 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), wide, wide_size);

    auto mbs_size = WideCharToMultiByte(CP_ACP, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    auto mbs = new char[mbs_size];
    memset(mbs, 0, mbs_size);
    WideCharToMultiByte(CP_ACP, 0, wide, -1, mbs, mbs_size, nullptr, nullptr); 

    auto cp949 = std::string(mbs);
    delete[] mbs;
    delete[] wide;
    return cp949;
#else
    size_t src_size = utf8.length();
    char* src = const_cast<char*>(utf8.data());

    size_t dst_size = src_size * 2;
    char* dst = new char[dst_size];
    char* dst_head = dst;
    std::memset(dst, 0, dst_size);

    iconv_t cd = iconv_open("EUC-KR", "UTF-8");
    iconv(cd, &src, &src_size, &dst, &dst_size);
    iconv_close(cd);

    auto cp949 = std::string(dst_head);
    delete[] dst_head;
    return cp949;
#endif
}

std::string utf8(const std::string& cp949)
{
    if(cp949.empty())
        return std::string();

#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_ACP, 0, cp949.c_str(), cp949.length(), nullptr, 0) + 1;
    auto wide = new wchar_t[wide_size];
    memset(wide, 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, cp949.c_str(), cp949.length(), wide, wide_size);

    auto mbs_size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    auto mbs = new char[mbs_size];
    memset(mbs, 0, mbs_size);
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, mbs, mbs_size, nullptr, nullptr); 

    auto utf8 = std::string(mbs);
    delete[] mbs;
    delete[] wide;
    return utf8;
#else
    size_t src_size = cp949.length();
    char* src = const_cast<char*>(cp949.data());

    size_t dst_size = src_size * 2;
    char* dst = new char[dst_size];
    char* dst_head = dst;
    std::memset(dst, 0, dst_size);

    iconv_t cd = iconv_open("UTF-8", "EUC-KR");
    iconv(cd, &src, &src_size, &dst, &dst_size);
    iconv_close(cd);

    auto utf8 = std::string(dst_head);
    delete[] dst_head;
    return utf8;
#endif
}

std::string CP949(const std::string& utf8)
{
#ifdef __linux__
    return cp949(utf8);
#else
    return utf8;
#endif
}

std::string UTF8(const std::string& cp949)
{
#ifdef __linux__
    return utf8(cp949);
#else
    return cp949;
#endif
}