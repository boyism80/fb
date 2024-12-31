#include <fb/game/protocol/dialog/dialog.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog::dialog(const fb::model::object&      object,
               const std::string&            message,
               bool                          button_prev,
               bool                          button_next,
               fb::game::dialog::interaction interaction) :
    object(object),
    message(message),
    button_prev(button_prev),
    button_next(button_next),
    interaction(interaction)
{ }

dialog::dialog(const fb::game::object&       object,
               const std::string&            message,
               bool                          button_prev,
               bool                          button_next,
               fb::game::dialog::interaction interaction) :
    dialog(object.based(), message, button_prev, button_next, interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);                                    // unknown
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction)); // interaction
    writer.write<uint32_t>(0x01);
    writer.write<uint8_t>(this->object.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->object.look);
    writer.write<uint8_t>(this->object.color);
    writer.write<uint8_t>(this->object.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->object.look);
    writer.write<uint8_t>(this->object.color);
    writer.write<uint32_t>(0x01);
    writer.write<uint8_t>(this->button_prev);
    writer.write<uint8_t>(this->button_next);
    writer.write<std::string, uint16_t>(this->message);
}
#else
async::task<void> dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
