#include <fb/game/protocol/dialog/dialog_input_ext.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_input_ext::dialog_input_ext(const fb::model::object&      obj,
                                   const std::string&            message,
                                   const std::string&            top,
                                   const std::string&            bottom,
                                   int                           maxlen,
                                   bool                          button_prev,
                                   uint32_t                      sequence,
                                   fb::game::dialog::interaction interaction) :
    obj(obj),
    message(message),
    top(top),
    bottom(bottom),
    maxlen(maxlen),
    button_prev(button_prev),
    sequence(sequence),
    interaction(interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog_input_ext::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x04);
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction));
    writer.write<uint32_t>(this->sequence);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
    writer.write<uint8_t>(this->obj.look > 0xBFFF ? 0x02 : 0x01);
    writer.write<uint16_t>(this->obj.look);
    writer.write<uint8_t>(this->obj.color);
    writer.write<uint32_t>(0x00000001);
    writer.write<uint8_t>(this->button_prev);
    writer.write<uint8_t>(0x00);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<std::string, uint8_t>(this->top);
    writer.write<uint8_t>(this->maxlen);
    writer.write<std::string, uint8_t>(this->bottom);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> dialog_input_ext::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
