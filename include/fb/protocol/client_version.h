#ifndef __FB_PROTOCOL_CLIENT_VERSION_H__
#define __FB_PROTOCOL_CLIENT_VERSION_H__

#include <cstdint>
#include <string_view>
#include <utility>

namespace fb::protocol {

enum class CLIENT_VERSION : uint16_t
{
    v550 = 550,
    v565 = 565,
    v651 = 651,
};

// 6.51 C2S transfer 0x10 trailing flag (byte_5E0866). Not a protocol version.
// OLD = 640x480 / tile24, NEW = 1024x768 / tile48.
enum class CLIENT_UI_MODE : uint8_t
{
    OLD = 0,
    NEW = 1,
};

inline constexpr bool is_supported(uint16_t packed)
{
    switch (packed)
    {
    case static_cast<uint16_t>(CLIENT_VERSION::v550):
    case static_cast<uint16_t>(CLIENT_VERSION::v565):
    case static_cast<uint16_t>(CLIENT_VERSION::v651):
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
    case CLIENT_VERSION::v651:
        return "6.51";
    default:
        return "unknown";
    }
}

inline std::string_view to_string(CLIENT_UI_MODE m)
{
    switch (m)
    {
    case CLIENT_UI_MODE::OLD:
        return "OLD";
    case CLIENT_UI_MODE::NEW:
        return "NEW";
    default:
        return "unknown";
    }
}

inline constexpr bool version_at_least(CLIENT_VERSION v, CLIENT_VERSION since)
{
    return static_cast<uint16_t>(v) >= static_cast<uint16_t>(since);
}

// Protocol template tags. Prefer FB_PROTOCOL_VERSION_TAGS(V) (since 5.50).
// For packets introduced later: FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565).
// Class named `version` uses the _NAMED variants (member cannot be called `version`).
#define FB_PROTOCOL_VERSION_TAGS(V)                                         \
    static constexpr CLIENT_VERSION version         = (V);                  \
    static constexpr CLIENT_VERSION available_since = CLIENT_VERSION::v550; \
    static constexpr bool           supported       = ::fb::protocol::version_at_least((V), available_since)

#define FB_PROTOCOL_VERSION_TAGS_SINCE(V, Since)               \
    static constexpr CLIENT_VERSION version         = (V);     \
    static constexpr CLIENT_VERSION available_since = (Since); \
    static constexpr bool           supported       = ::fb::protocol::version_at_least((V), available_since)

#define FB_PROTOCOL_VERSION_TAGS_NAMED(V)                                    \
    static constexpr CLIENT_VERSION protocol_version = (V);                  \
    static constexpr CLIENT_VERSION available_since  = CLIENT_VERSION::v550; \
    static constexpr bool           supported        = ::fb::protocol::version_at_least((V), available_since)

#define FB_PROTOCOL_VERSION_TAGS_NAMED_SINCE(V, Since)          \
    static constexpr CLIENT_VERSION protocol_version = (V);     \
    static constexpr CLIENT_VERSION available_since  = (Since); \
    static constexpr bool           supported        = ::fb::protocol::version_at_least((V), available_since)

template <typename ProtocolType>
constexpr bool protocol_supported()
{
    if constexpr (requires { ProtocolType::supported; })
        return ProtocolType::supported;
    else
        return true;
}

template <typename ProtocolType>
constexpr CLIENT_VERSION protocol_available_since()
{
    if constexpr (requires { ProtocolType::available_since; })
        return ProtocolType::available_since;
    else
        return CLIENT_VERSION::v550;
}

// C2S version 0x00: v565/v651 append u16 build/LS after nation.
inline constexpr bool has_version_build_field(CLIENT_VERSION v)
{
    return v == CLIENT_VERSION::v565 || v == CLIENT_VERSION::v651;
}

// Dispatch on a runtime CLIENT_VERSION to a compile-time V (C++20).
template <typename F>
decltype(auto) visit_client_version(CLIENT_VERSION v, F&& f)
{
    switch (v)
    {
    case CLIENT_VERSION::v550:
        return std::forward<F>(f).template operator()<CLIENT_VERSION::v550>();
    case CLIENT_VERSION::v565:
        return std::forward<F>(f).template operator()<CLIENT_VERSION::v565>();
    case CLIENT_VERSION::v651:
        return std::forward<F>(f).template operator()<CLIENT_VERSION::v651>();
    default:
        return std::forward<F>(f).template operator()<CLIENT_VERSION::v550>();
    }
}

// Session missing or version not yet established → v550.
template <typename SessionData>
CLIENT_VERSION client_version_or_default(const SessionData* data)
{
    if (data == nullptr)
        return CLIENT_VERSION::v550;

    if constexpr (requires { data->client_version; })
        return data->client_version;
    else
        return CLIENT_VERSION::v550;
}

} // namespace fb::protocol

#endif
