#ifndef __FB_PROTOCOL_CLIENT_VERSION_H__
#define __FB_PROTOCOL_CLIENT_VERSION_H__

#include <cstdint>
#include <string_view>

namespace fb::protocol {

enum class CLIENT_VERSION : uint16_t
{
    v550 = 550,
    v565 = 565,
};

inline bool try_parse(uint16_t raw, CLIENT_VERSION& out)
{
    switch (raw)
    {
    case 550:
        out = CLIENT_VERSION::v550;
        return true;
    case 565:
        out = CLIENT_VERSION::v565;
        return true;
    default:
        return false;
    }
}

inline std::string_view to_string(CLIENT_VERSION v)
{
    switch (v)
    {
    case CLIENT_VERSION::v550:
        return "5.50";
    case CLIENT_VERSION::v565:
        return "5.65";
    default:
        return "unknown";
    }
}

} // namespace fb::protocol

#endif
