#ifndef __PROTOCOL_GAME_UPDATE_SLOT_H__
#define __PROTOCOL_GAME_UPDATE_SLOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/item.h>
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class item_update_slot : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x37;

public:
#ifndef BOT
    const fb::game::character& me;
    const EQUIPMENT_PARTS      parts;
#else

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