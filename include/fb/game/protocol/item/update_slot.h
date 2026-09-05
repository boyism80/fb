#ifndef __PROTOCOL_GAME_UPDATE_SLOT_H__
#define __PROTOCOL_GAME_UPDATE_SLOT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/item.h>
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_update_slot : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x37;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const fb::game::character& me;
    const EQUIPMENT_PARTS      parts;
    std::string                unknown_name_b;
    uint32_t                   unknown_u32 = 0;
#else
    EQUIPMENT_PARTS parts;
    uint16_t        look;
    uint8_t         color;
    std::string     name;
    std::string     unknown_name_b;
    uint32_t        unknown_u32;
#endif

public:
#ifndef BOT
    item_update_slot(const fb::game::character& me, EQUIPMENT_PARTS parts);
#else
    item_update_slot() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using item_update_slot_v550 = item_update_slot<CLIENT_VERSION::v550>;
using item_update_slot_v565 = item_update_slot<CLIENT_VERSION::v565>;
using item_update_slot_v651 = item_update_slot<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
