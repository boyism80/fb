#include <fb/game/protocol/dialog/dialog.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog::dialog(const fb::model::object&      object,
               const std::string&            message,
               bool                          button_prev,
               bool                          button_next,
               uint32_t                      sequence,
               fb::game::dialog::interaction interaction) :
    object(object),
    message(message),
    button_prev(button_prev),
    button_next(button_next),
    sequence(sequence),
    interaction(interaction)
{ }

dialog::dialog(const fb::game::object&       object,
               const std::string&            message,
               bool                          button_prev,
               bool                          button_next,
               uint32_t                      sequence,
               fb::game::dialog::interaction interaction) :
    dialog(object.based(), message, button_prev, button_next, sequence, interaction)
{ }
#endif

#ifndef BOT
async::task<void> dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);                                    // unknown
    writer.write<uint8_t>(static_cast<uint8_t>(this->interaction)); // interaction
    writer.write<uint32_t>(this->sequence);
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
    reader.read<uint8_t>(); // 0x00
    this->interaction = reader.read<uint8_t>();
    this->sequence    = reader.read<uint32_t>();
    reader.read<uint8_t>(); // obj type flag
    reader.read<uint8_t>(); // 0x01
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    reader.read<uint32_t>(); // 0x01
    this->button_prev = reader.read<uint8_t>();
    this->button_next = reader.read<uint8_t>();
    this->message     = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
