#ifndef __PROTOCOL_GAME_ITEM_THROW_CONFIRM_H__
#define __PROTOCOL_GAME_ITEM_THROW_CONFIRM_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class item_throw_confirm : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4E;

public:
#ifndef BOT
    const uint8_t inventory_slot;
#else
    uint8_t inventory_slot;
#endif

public:
#ifndef BOT
    explicit item_throw_confirm(uint8_t inventory_slot) :
        inventory_slot(inventory_slot)
    { }
#else
    item_throw_confirm() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
