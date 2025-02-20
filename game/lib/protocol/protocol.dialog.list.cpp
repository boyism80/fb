#include <fb/game/protocol/dialog/dialog_list.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_list::dialog_list(const fb::model::npc&           npc,
                         const std::vector<std::string>& lists,
                         const std::string&              message,
                         bool                            button_prev,
                         fb::game::dialog::interaction   interaction) :
    npc(npc),
    lists(lists),
    message(message),
    button_prev(button_prev),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(2);
    writer.write<uint8_t>(static_cast<uint8_t>(interaction));
    writer.write<uint32_t>(0x01);
    writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->npc.look);
    writer.write<uint8_t>(this->npc.color);
    writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->npc.look);
    writer.write<uint8_t>(this->npc.color);
    writer.write<uint32_t>(1);
    writer.write<uint8_t>(this->button_prev); // button prev
    writer.write<uint8_t>(1);
    writer.write<std::string, uint16_t>(message);

    writer.write<uint8_t>(lists.size());
    for (int i = 0; i < lists.size(); i++)
    {
        writer.write<std::string>(lists[i]);
    }
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> dialog_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
