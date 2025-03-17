#include <fb/game/protocol/dialog/dialog_item.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_item::dialog_item(const fb::model::object&            obj,
                         const fb::game::dialog::item_pairs& items,
                         const std::string&                  message,
                         uint32_t                            sequence,
                         uint16_t                            pursuit,
                         fb::game::dialog::interaction       interaction) :
    obj(obj),
    items(items),
    message(message),
    sequence(sequence),
    pursuit(pursuit),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_item::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x04);
    writer.write<uint8_t>(static_cast<uint8_t>(interaction));
    writer.write<uint32_t>(this->sequence);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
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
async::task<void> dialog_item::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
