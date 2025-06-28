#ifndef __PROTOCOL_GAME_WORLD_H__
#define __PROTOCOL_GAME_WORLD_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class map_world : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x3F;

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
    map_world() = default;

public:
#ifdef BOT // bot only
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else // server only
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif