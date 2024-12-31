#include <fb/game/protocol/item/item_update_slot.h>

namespace fb::protocol::game::response {

#ifndef BOT
item_update_slot::item_update_slot(const fb::game::character& me, EQUIPMENT_PARTS parts) :
    me(me),
    parts(parts)
{ }
#endif

#ifndef BOT
async::task<void> item_update_slot::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    fb::game::item* item;

    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        item = this->me.items.weapon();
        break;

    case EQUIPMENT_PARTS::ARMOR:
        item = this->me.items.armor();
        break;

    case EQUIPMENT_PARTS::SHIELD:
        item = this->me.items.shield();
        break;

    case EQUIPMENT_PARTS::HELMET:
        item = this->me.items.helmet();
        break;

    case EQUIPMENT_PARTS::LEFT_HAND:
        item = this->me.items.ring(EQUIPMENT_POSITION::LEFT);
        break;

    case EQUIPMENT_PARTS::RIGHT_HAND:
        item = this->me.items.ring(EQUIPMENT_POSITION::RIGHT);
        break;

    case EQUIPMENT_PARTS::LEFT_AUX:
        item = this->me.items.auxiliary(EQUIPMENT_POSITION::LEFT);
        break;

    case EQUIPMENT_PARTS::RIGHT_AUX:
        item = this->me.items.auxiliary(EQUIPMENT_POSITION::RIGHT);
        break;

    default:
        co_return;
    }

    if (item == nullptr)
        co_return;

    writer.write<uint8_t>(header);
    writer.write<uint16_t>(item->look());
    writer.write<uint8_t>(item->color());
    writer.write<std::string, uint8_t>(item->name());
}
#else
async::task<void> item_update_slot::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
