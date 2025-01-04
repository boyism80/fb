#include <fb/game/protocol/dialog/dialog_input.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_input::dialog_input(const fb::model::npc&         npc,
                           const std::string&            message,
                           fb::game::dialog::interaction interaction) :
    npc(npc),
    message(message),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_input::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x03);
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
    writer.write<uint32_t>(0x01);
    writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->npc.look);
    writer.write<uint8_t>(this->npc.color);
    writer.write<uint8_t>(this->npc.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->npc.look);
    writer.write<uint8_t>(this->npc.color);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> dialog_input::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
