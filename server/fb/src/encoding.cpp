#ifdef _WIN32
#include <Windows.h>
#endif
#include <fb/encoding.h>

std::string fb::cp949(std::string_view utf8)
{
    if (utf8.empty())
        return std::string();

#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.length()), nullptr, 0) + 1;
    auto wide      = std::vector<wchar_t>(wide_size);
    memset(wide.data(), 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.length()), wide.data(), wide_size);

    auto mbs_size = WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    auto mbs      = std::vector<char>(mbs_size);
    memset(mbs.data(), 0, mbs_size);
    WideCharToMultiByte(CP_ACP, 0, wide.data(), -1, mbs.data(), mbs_size, nullptr, nullptr);

    return std::string(mbs.data());
#else
    size_t src_size = utf8.length();
    char*  src      = const_cast<char*>(utf8.data());

    size_t dst_size = src_size * 2;
    auto   dst_vec  = std::vector<char>(dst_size, 0);
    char*  dst      = dst_vec.data();

    iconv_t cd = iconv_open("CP949", "UTF-8");
    iconv(cd, &src, &src_size, &dst, &dst_size);
    iconv_close(cd);

    return std::string(dst_vec.data());
#endif
}

std::string fb::utf8(std::string_view cp949)
{
    if (cp949.empty())
        return std::string();

#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_ACP, 0, cp949.data(), static_cast<int>(cp949.length()), nullptr, 0) + 1;
    auto wide      = std::vector<wchar_t>(wide_size);
    memset(wide.data(), 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, cp949.data(), static_cast<int>(cp949.length()), wide.data(), wide_size);

    auto mbs_size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    auto mbs      = std::vector<char>(mbs_size);
    memset(mbs.data(), 0, mbs_size);
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, mbs.data(), mbs_size, nullptr, nullptr);

    return std::string(mbs.data());
#else
    size_t src_size = cp949.length();
    char*  src      = const_cast<char*>(cp949.data());

    size_t dst_size = src_size * 2;
    auto   dst_vec  = std::vector<char>(dst_size, 0);
    char*  dst      = dst_vec.data();

    iconv_t cd = iconv_open("UTF-8", "CP949");
    iconv(cd, &src, &src_size, &dst, &dst_size);
    iconv_close(cd);

    return std::string(dst_vec.data());
#endif
}

std::string fb::CP949(std::string_view utf8, PLATFORM p)
{
    switch (p)
    {
    case PLATFORM::BOTH:
        return cp949(utf8);

    case PLATFORM::WINDOWS:
#ifdef _WIN32
        return cp949(utf8);
#else
        return std::string(utf8);
#endif

    case PLATFORM::LINUX:
#ifdef __linux__
        return cp949(utf8);
#else
        return std::string(utf8);
#endif

    default:
        return std::string(utf8);
    }
}

std::string fb::UTF8(std::string_view cp949, PLATFORM p)
{
    switch (p)
    {
    case PLATFORM::BOTH:
        return utf8(cp949);

    case PLATFORM::WINDOWS:
#ifdef _WIN32
        return utf8(cp949);
#else
        return std::string(cp949);
#endif

    case PLATFORM::LINUX:
#ifdef __linux__
        return utf8(cp949);
#else
        return std::string(cp949);
#endif

    default:
        return std::string(cp949);
    }
}

std::wstring fb::W(std::string_view m)
{
#ifdef _WIN32
    auto wide_size = MultiByteToWideChar(CP_ACP, 0, m.data(), static_cast<int>(m.length()), nullptr, 0) + 1;
    auto wide      = std::vector<wchar_t>(wide_size);
    memset(wide.data(), 0, wide_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, m.data(), static_cast<int>(m.length()), wide.data(), wide_size);

    return std::wstring(wide.data());
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(m.data(), m.data() + m.length());
#endif
}

std::string fb::M(const std::wstring& w)
{
#ifdef _WIN32
    auto mbs_size = WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    auto mbs      = std::vector<char>(mbs_size);
    memset(mbs.data(), 0, mbs_size);
    WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, mbs.data(), mbs_size, nullptr, nullptr);

    return std::string(mbs.data());
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(w);
#endif
}

std::string fb::name_with(std::string_view name)
{
    return name_with(name, {"을", "를"});
}

std::string fb::name_with(std::string_view name, const std::pair<std::string, std::string>& postfix)
{
    auto wide = W(name);
    auto last = wide.at(wide.size() - 1);
    if (last < 0xAC00 || last > 0xD7A3)
        return std::string(name) + postfix.second;

    if ((last - 0xAC00) % 28 > 0)
        return std::string(name) + postfix.first;
    else
        return std::string(name) + postfix.second;
}

bool fb::assert_korean(std::string_view str)
{
    for (auto ch : str)
    {
        if ((ch & 0x80) == 0)
            return false;
    }

    return true;
}

std::string fb::url_encode(std::string_view value)
{
    auto escaped = std::ostringstream{};
    escaped.fill('0');
    escaped << std::hex;

    for (auto i = value.begin(), n = value.end(); i != n; ++i)
    {
        auto c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (std::isalnum(static_cast<uint8_t>(c)))
        {
            escaped << c;
            continue;
        }

        switch (c)
        {
        case '-':
        case '_':
        case '.':
        case '~':
        case '/':
        case '?':
        case '=':
        case '&':
        case ':':
            escaped << c;
            break;

        default:
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<uint8_t>(c));
            escaped << std::nouppercase;
            break;
        }
    }

    return escaped.str();
}
