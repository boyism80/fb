#ifdef _WIN32
#include <Windows.h>
#endif
#include <fb/encoding.h>
#include <random.h>

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

namespace {

bool is_cp949_lead(uint8_t b)
{
    return b >= 0x81 && b <= 0xFE;
}

bool is_cp949_trail(uint8_t b)
{
    return (b >= 0x41 && b <= 0x5A) || (b >= 0x61 && b <= 0x7A) || (b >= 0x81 && b <= 0xFE);
}

bool utf8_first_codepoint(std::string_view u8, char32_t& out)
{
    if (u8.empty())
        return false;

    auto b0 = static_cast<uint8_t>(u8[0]);
    if (b0 < 0x80)
    {
        out = b0;
        return true;
    }
    else if ((b0 & 0xE0) == 0xC0)
    {
        if (u8.size() < 2)
            return false;
        auto b1 = static_cast<uint8_t>(u8[1]);
        if ((b1 & 0xC0) != 0x80)
            return false;
        out = (static_cast<char32_t>(b0 & 0x1F) << 6) | (b1 & 0x3F);
        return true;
    }
    else if ((b0 & 0xF0) == 0xE0)
    {
        if (u8.size() < 3)
            return false;
        auto b1 = static_cast<uint8_t>(u8[1]);
        auto b2 = static_cast<uint8_t>(u8[2]);
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80)
            return false;
        out = (static_cast<char32_t>(b0 & 0x0F) << 12) | (static_cast<char32_t>(b1 & 0x3F) << 6) | (b2 & 0x3F);
        return true;
    }
    else if ((b0 & 0xF8) == 0xF0)
    {
        if (u8.size() < 4)
            return false;
        auto b1 = static_cast<uint8_t>(u8[1]);
        auto b2 = static_cast<uint8_t>(u8[2]);
        auto b3 = static_cast<uint8_t>(u8[3]);
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80)
            return false;
        out = (static_cast<char32_t>(b0 & 0x07) << 18) | (static_cast<char32_t>(b1 & 0x3F) << 12) |
              (static_cast<char32_t>(b2 & 0x3F) << 6) | (b3 & 0x3F);
        return true;
    }

    return false;
}

bool codepoint_is_hangul_syllable(char32_t c)
{
    return c >= 0xAC00 && c <= 0xD7A3;
}

bool codepoint_is_hangul_jamo(char32_t c)
{
    return (c >= 0x1100 && c <= 0x11FF) || (c >= 0x3131 && c <= 0x318E) || (c >= 0xA960 && c <= 0xA97F) ||
           (c >= 0xD7B0 && c <= 0xD7FF);
}

bool cp949_pair_to_codepoint(uint8_t lead, uint8_t trail, char32_t& out)
{
    char buf[2] = {static_cast<char>(lead), static_cast<char>(trail)};
    auto u8     = fb::utf8(std::string_view(buf, 2));
    return utf8_first_codepoint(u8, out);
}

// Completed Hangul syllable in CP949 (KS X 1001 + extended). Rejects jamo.
bool is_hangul_syllable_bytes(uint8_t lead, uint8_t trail)
{
    if (!is_cp949_lead(lead) || !is_cp949_trail(trail))
        return false;

    char32_t cp = 0;
    if (!cp949_pair_to_codepoint(lead, trail, cp))
        return false;
    return codepoint_is_hangul_syllable(cp);
}

} // namespace

bool fb::assert_korean(std::string_view cp949, bool completed)
{
    if (cp949.empty())
        return false;

    for (size_t i = 0; i < cp949.size();)
    {
        auto lead = static_cast<uint8_t>(cp949[i]);
        if (lead < 0x80)
            return false;
        if (i + 1 >= cp949.size())
            return false;

        auto trail = static_cast<uint8_t>(cp949[i + 1]);
        if (completed)
        {
            if (!is_hangul_syllable_bytes(lead, trail))
                return false;
        }
        else
        {
            if (!is_cp949_lead(lead) || !is_cp949_trail(trail))
                return false;

            char32_t cp = 0;
            if (!cp949_pair_to_codepoint(lead, trail, cp))
                return false;
            if (!codepoint_is_hangul_syllable(cp) && !codepoint_is_hangul_jamo(cp))
                return false;
        }

        i += 2;
    }

    return true;
}

std::string fb::delirious(std::string_view message)
{
    if (message.empty())
        return std::string();

    auto bytes = CP949(message);
    auto out   = std::string{};
    out.reserve(bytes.size());

    for (size_t i = 0; i < bytes.size();)
    {
        auto lead = static_cast<uint8_t>(bytes[i]);
        if (lead < 0x80 || i + 1 >= bytes.size())
        {
            out.push_back(static_cast<char>(lead));
            i += 1;
            continue;
        }

        auto trail = static_cast<uint8_t>(bytes[i + 1]);
        if (!is_cp949_lead(lead) || !is_cp949_trail(trail))
        {
            out.push_back(static_cast<char>(lead));
            i += 1;
            continue;
        }

        auto choice = random(0, 3);
        if (choice == 0)
        {
            out.push_back(static_cast<char>(lead));
            out.push_back(static_cast<char>(trail));
        }
        else if (choice == 1 && lead >= 0xA1 && lead <= 0xFE && trail >= 0xA1 && trail <= 0xFE &&
                 is_hangul_syllable_bytes(trail, lead))
        {
            // Byte-swap only when swapped KS cell decodes to a Hangul syllable
            out.push_back(static_cast<char>(trail));
            out.push_back(static_cast<char>(lead));
        }
        else if (choice == 1)
        {
            out.push_back(static_cast<char>(lead));
            out.push_back(static_cast<char>(trail));
        }
        else if (choice == 2)
        {
            out.push_back('*');
        }
        else
        {
            out.push_back('?');
        }

        i += 2;
    }

    return UTF8(out);
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
