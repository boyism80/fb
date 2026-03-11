#ifndef __PROTOCOL_GAME_UNKNOWN_26_H__
#define __PROTOCOL_GAME_UNKNOWN_26_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class unknown_26 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x26;

public:
#ifndef BOT
    const uint8_t flags;
    const int16_t pos_x;
    const int16_t pos_y;
    const int16_t map_rel_x;
    const int16_t map_rel_y;
    const uint8_t zone_slot;
#else
    uint8_t flags;
    int16_t pos_x;
    int16_t pos_y;
    int16_t map_rel_x;
    int16_t map_rel_y;
    uint8_t zone_slot;
#endif

public:
#ifndef BOT
    unknown_26(uint8_t flags, int16_t pos_x, int16_t pos_y, int16_t map_rel_x, int16_t map_rel_y, uint8_t zone_slot) :
        flags(flags),
        pos_x(pos_x),
        pos_y(pos_y),
        map_rel_x(map_rel_x),
        map_rel_y(map_rel_y),
        zone_slot(zone_slot)
    { }
#else
    unknown_26() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
