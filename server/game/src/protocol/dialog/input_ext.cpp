#include <fb/game/protocol/dialog/input_ext.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_input_ext::dialog_input_ext(const fb::model::object& obj,
                                   std::string_view         message,
                                   std::string_view         top,
                                   std::string_view         bottom,
                                   int                      maxlen,
                                   bool                     button_prev,
                                   uint32_t                 oid) :
    appearance(fb::game::appearance_factory::create(obj)),
    message(std::string(message)),
    top(std::string(top)),
    bottom(std::string(bottom)),
    maxlen(maxlen),
    button_prev(button_prev),
    oid(oid)
{ }

dialog_input_ext::dialog_input_ext(const fb::game::object& object,
                                   std::string_view        message,
                                   std::string_view        top,
                                   std::string_view        bottom,
                                   int                     maxlen,
                                   bool                    button_prev,
                                   uint32_t                oid) :
    appearance(fb::game::appearance_factory::create(object)),
    message(std::string(message)),
    top(std::string(top)),
    bottom(std::string(bottom)),
    maxlen(maxlen),
    button_prev(button_prev),
    oid(oid)
{ }
#endif

#ifndef BOT
void dialog_input_ext::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(1);
    writer.write<bool>(this->button_prev);
    writer.write<uint8_t>(0x00);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<std::string, uint8_t>(this->top);
    writer.write<uint8_t>(this->maxlen);
    writer.write<std::string, uint8_t>(this->bottom);
    writer.write<uint8_t>(0x00);
}
#else
void dialog_input_ext::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    this->type_echo = reader.read<uint8_t>();
    this->oid       = reader.read<uint32_t>();
    reader.read<uint8_t>(); // obj type flag
    reader.read<uint8_t>(); // 0x01
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    reader.read<uint32_t>(); // 0x00000001
    this->button_prev = reader.read<bool>();
    reader.read<uint8_t>(); // 0x00
    this->message = reader.read<std::string, uint16_t>();
    this->top     = reader.read<std::string, uint8_t>();
    this->maxlen  = reader.read<uint8_t>();
    this->bottom  = reader.read<std::string, uint8_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
