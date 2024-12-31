#include <fb/game/protocol/dialog/dialog_item.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_item::dialog_item(const fb::model::npc&               npc,
                         const fb::game::dialog::item_pairs& items,
                         const std::string&                  message,
                         uint16_t                            pursuit,
                         fb::game::dialog::interaction       interaction) :
    npc(npc),
    items(items),
    message(message),
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
    writer.write<uint32_t>(0x01);
    writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->npc.look);
    writer.write<uint8_t>(this->npc.color);
    writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->npc.look);
    writer.write<uint8_t>(this->npc.color);
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
