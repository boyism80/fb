#ifndef __PROTOCOL_GAME_REPUTATION_H__
#define __PROTOCOL_GAME_REPUTATION_H__

#include <fb/protocol/client_version.h>
#include <fb/protocol/header.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace fb::protocol::game::request {

template <CLIENT_VERSION V>
class reputation : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x46;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifdef BOT
    const uint8_t     subtype = 0;
    const std::string name;
    const bool        raise = false;
#else
    uint8_t     subtype = 0;
    std::string name;
    bool        raise = false;
#endif

public:
#ifdef BOT
    reputation(std::string_view name, bool raise);
#else
    reputation() = default;
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
