#include <fb/core/encoding.h>

std::string cp949(const std::string& utf8)
{
    if(utf8.empty())
        return std::string();

#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), nullptr, 0) + 1;
    auto wide = std::vector<wchar_t>(wide_size);
    memset(wide.data(), 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), wide.data(), wide_size);

    auto mbs_size = WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    auto mbs = std::vector<char>(mbs_size);
    memset(mbs.data(), 0, mbs_size);
    WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, mbs.data(), mbs_size, nullptr, nullptr);

    return std::string(mbs.data());
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
    auto wide = std::vector<wchar_t>(wide_size);
    memset(wide.data(), 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, cp949.c_str(), cp949.length(), wide.data(), wide_size);

    auto mbs_size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    auto mbs = std::vector<char>(mbs_size);
    memset(mbs.data(), 0, mbs_size);
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, mbs.data(), mbs_size, nullptr, nullptr); 

    return std::string(mbs.data());
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
    auto wide = std::vector<wchar_t>(wide_size);
    memset(wide.data(), 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, m.c_str(), m.length(), wide.data(), wide_size);

    return std::wstring(wide.data());
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(m);
#endif
}

std::string M(const std::wstring& w)
{
#ifdef _WIN32
    auto mbs_size = WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    auto mbs = std::vector<char>(mbs_size);
    memset(mbs.data(), 0, mbs_size);
    WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, mbs.data(), mbs_size, nullptr, nullptr);
    
    return std::string(mbs.data());
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

// 참고자료
// https://gala04.tistory.com/entry/%EC%A0%9C%EB%AA%A9%EC%9D%84-%EC%9E%85%EB%A0%A5%ED%95%B4-%EC%A3%BC%EC%84%B8%EC%9A%94
bool assert_korean(const std::string& str)
{
    auto len = str.length();
    if (len % 2 > 0)
        return false;

    auto raw = str.c_str();
    for (int i = 0; i < len; i += 2)
    {
        uint8_t e1 = raw[i + 0];
        uint8_t e2 = raw[i + 1];

        if (isascii(e1))
            return false;

        if (e1 < 0xB0 || e1 > 0xC8)
            return false;

        if (e2 < 0xA1 || e2 > 0xFE)
            return false;
    }

    return true;
}