#include <fb/core/encoding.h>

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
    return std::move(cp949);
#else
    size_t src_size = utf8.length();
    char* src = const_cast<char*>(utf8.data());

    size_t dst_size = src_size * 2;
    char* dst = new char[dst_size];
    char* dst_head = dst;
    std::memset(dst, 0, dst_size);

    iconv_t cd = iconv_open("CP949", "UTF-8");
    iconv(cd, &src, &src_size, &dst, &dst_size);
    iconv_close(cd);

    auto cp949 = std::string(dst_head);
    delete[] dst_head;
    return std::move(cp949);
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
    return std::move(utf8);
#else
    size_t src_size = cp949.length();
    char* src = const_cast<char*>(cp949.data());

    size_t dst_size = src_size * 2;
    char* dst = new char[dst_size];
    char* dst_head = dst;
    std::memset(dst, 0, dst_size);

    iconv_t cd = iconv_open("UTF-8", "CP949");
    iconv(cd, &src, &src_size, &dst, &dst_size);
    iconv_close(cd);

    auto utf8 = std::string(dst_head);
    delete[] dst_head;
    return std::move(utf8);
#endif
}

std::string CP949(const std::string& utf8, PLATFORM p)
{
    switch(p)
    {
    case PLATFORM::Both:
        return cp949(utf8);

    case PLATFORM::Windows:
#ifdef _WIN32
        return cp949(utf8);
#else
        return utf8;
#endif

    case PLATFORM::Linux:
#ifdef __linux__
        return cp949(utf8);
#else
        return utf8;
#endif

    default:
        return utf8;
    }
}

std::string UTF8(const std::string& cp949, PLATFORM p)
{
    switch(p)
    {
    case PLATFORM::Both:
        return utf8(cp949);

    case PLATFORM::Windows:
#ifdef _WIN32
        return utf8(cp949);
#else
        return cp949;
#endif

    case PLATFORM::Linux:
#ifdef __linux__
        return utf8(cp949);
#else
        return cp949;
#endif

    default:
        return cp949;
    }
}

std::wstring W(const std::string& m)
{
#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_ACP, 0, m.c_str(), m.length(), nullptr, 0) + 1;
    auto wide = new wchar_t[wide_size];
    memset(wide, 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, m.c_str(), m.length(), wide, wide_size);

    auto dst = std::wstring(wide);
    delete[] wide;
    return dst;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(m);
#endif
}

std::string M(const std::wstring& w)
{
#ifdef _WIN32
    auto mbs_size = WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    auto mbs = new char[mbs_size];
    memset(mbs, 0, mbs_size);
    WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, mbs, mbs_size, nullptr, nullptr);
    
    auto dst = std::string(mbs);
    delete[] mbs;
    return dst;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(w);
#endif
}

std::string name_with(const std::string& name)
{
    return name_with(name, {"을", "를"});
}

std::string name_with(const std::string& name, const std::pair<std::string, std::string>& postfix)
{
    auto wide = W(name);
    auto last = wide.at(wide.size() - 1);
    if (last < 0xAC00 || last > 0xD7A3)
        return name;

    if ((last - 0xAC00) % 28 > 0)
        return name + postfix.first;
    else
        return name + postfix.second;
}