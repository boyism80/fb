#include <fb/game/protocol/dialog/dialog_input_ext.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_input_ext::dialog_input_ext(fb::model::model&             model,
                                   const fb::model::object&      obj,
                                   const std::string&            message,
                                   const std::string&            top,
                                   const std::string&            bottom,
                                   int                           maxlen,
                                   bool                          button_prev,
                                   uint32_t                      oid,
                                   fb::game::dialog::interaction interaction) :
    portrait(fb::game::dialog::portrait_factory::create(model, obj)),
    message(message),
    top(top),
    bottom(bottom),
    maxlen(maxlen),
    button_prev(button_prev),
    oid(oid),
    interaction(interaction)
{ }

dialog_input_ext::dialog_input_ext(const fb::game::object&       object,
                                   const std::string&            message,
                                   const std::string&            top,
                                   const std::string&            bottom,
                                   int                           maxlen,
                                   bool                          button_prev,
                                   uint32_t                      oid,
                                   fb::game::dialog::interaction interaction) :
    portrait(fb::game::dialog::portrait_factory::create(object)),
    message(message),
    top(top),
    bottom(bottom),
    maxlen(maxlen),
    button_prev(button_prev),
    oid(oid),
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
    writer.write<uint32_t>(this->oid);
    this->portrait->serialize(writer);
    writer.write<uint32_t>(1);
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
    reader.read<uint32_t>(); // 0x00000001
    this->button_prev = reader.read<uint8_t>();
    reader.read<uint8_t>(); // 0x00
    this->message = reader.read<std::string, uint16_t>();
    this->top     = reader.read<std::string, uint8_t>();
    this->maxlen  = reader.read<uint8_t>();
    this->bottom  = reader.read<std::string, uint8_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
