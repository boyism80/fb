#ifndef __PROTOCOL_GAME_WORLD_H__
#define __PROTOCOL_GAME_WORLD_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class map_world : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x3F;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT // server only
    uint16_t value;
    uint16_t before;
    uint16_t after;
#else // bot only
    const uint16_t value;
    const uint16_t before;
    const uint16_t after;
#endif

public:
#ifndef BOT
    map_world() = default;
#else
    map_world(uint16_t value, uint16_t before, uint16_t after);
#endif

public:
#ifdef BOT // bot only
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else // server only
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif