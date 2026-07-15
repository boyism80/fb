#include <fb/game/protocol/dialog/item.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_item::dialog_item(const fb::model::object&            obj,
                         const fb::game::dialog::item_pairs& items,
                         std::string_view                    message,
                         uint32_t                            oid,
                         uint16_t                            pursuit,
                         fb::game::dialog::interaction       interaction) :
    appearance(fb::game::appearance_factory::create(obj)),
    items(items),
    message(std::string(message)),
    oid(oid),
    pursuit(pursuit),
    interaction(interaction)
{ }

dialog_item::dialog_item(const fb::game::object&             object,
                         const fb::game::dialog::item_pairs& items,
                         std::string_view                    message,
                         uint32_t                            oid,
                         uint16_t                            pursuit,
                         fb::game::dialog::interaction       interaction) :
    appearance(fb::game::appearance_factory::create(object)),
    items(items),
    message(std::string(message)),
    oid(oid),
    pursuit(pursuit),
    interaction(interaction)
{ }
#endif

#ifndef BOT
void dialog_item::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x04);
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(this->pursuit);
    writer.write<uint16_t>((uint16_t)this->items.size());

    for (auto& [item, value] : this->items)
    {
        writer.write<uint16_t>(item.look);
        writer.write<uint8_t>(item.color);
        writer.write<uint32_t>(value);
        writer.write<std::string>(item.name);
        writer.write<std::string>(item.desc);
    }

    writer.write<uint8_t>(0x00);
}
#else
void dialog_item::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x04
    this->interaction = reader.read<uint8_t>();
    this->oid         = reader.read<uint32_t>();
    reader.read<uint8_t>(); // obj type flag
    reader.read<uint8_t>(); // 0x01
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    this->message       = reader.read<std::string, uint16_t>();
    this->pursuit       = reader.read<uint16_t>();
    uint16_t item_count = reader.read<uint16_t>();

    this->items.clear();
    for (int i = 0; i < item_count; i++)
    {
        item_data item;
        item.look  = reader.read<uint16_t>();
        item.color = reader.read<uint8_t>();
        item.value = reader.read<uint32_t>();
        item.name  = reader.read<std::string, uint8_t>();
        item.desc  = reader.read<std::string, uint8_t>();
        this->items.push_back(item);
    }
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
