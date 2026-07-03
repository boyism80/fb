#ifndef __PROTOCOL_GAME_UPDATE_SLOT_H__
#define __PROTOCOL_GAME_UPDATE_SLOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/item.h>
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class item_update_slot : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x37;

public:
#ifndef BOT
    const fb::game::character& me;
    const EQUIPMENT_PARTS      parts;
#else
    uint16_t    look;
    uint8_t     color;
    std::string name;
#endif

public:
#ifndef BOT
    item_update_slot(const fb::game::character& me, EQUIPMENT_PARTS parts);
#else
    item_update_slot() = default;
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