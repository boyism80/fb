#include <fb/game/protocol/item/update.h>

namespace fb::protocol::game::response {

#ifndef BOT
item_update::item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count, bool valid) :
    index(index),
    look(look),
    color(color),
    name(std::move(name)),
    count(count),
    valid(valid)
{ }

item_update item_update::from_inventory(const fb::game::character& me, uint8_t index)
{
    auto item = me.items.at(index);
    if (item == nullptr)
        return item_update(index, 0, 0, std::string{}, 0, false);

    return item_update(index,
                       item->look(),
                       item->color(),
                       item->inven_name(),
                       static_cast<uint32_t>(item->count() - item->trade_count()),
                       true);
}

item_update::item_update(const fb::game::character& me, uint8_t index) :
    item_update(from_inventory(me, index))
{ }

item_update::item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count) :
    item_update(index, look, color, std::move(name), count, true)
{ }

void item_update::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    if (this->valid == false)
        return;

    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint16_t>(this->look);
    writer.write<uint8_t>(this->color);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<uint32_t>(this->count);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x00);
}
#else
void item_update::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1; // Client sends 1-based index
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    this->name  = reader.read<std::string, uint8_t>();
    this->count = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
