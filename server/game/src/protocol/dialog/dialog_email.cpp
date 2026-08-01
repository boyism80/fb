#include <fb/game/protocol/dialog/dialog_email.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_email::dialog_email(const fb::model::object& obj,
                           std::string_view         message,
                           std::string_view         str1,
                           std::string_view         str2,
                           bool                     button_prev,
                           bool                     button_next,
                           uint32_t                 oid) :
    appearance(fb::game::appearance_factory::create(obj)),
    message(std::string(message)),
    str1(std::string(str1)),
    str2(std::string(str2)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

dialog_email::dialog_email(const fb::game::object& object,
                           std::string_view        message,
                           std::string_view        str1,
                           std::string_view        str2,
                           bool                    button_prev,
                           bool                    button_next,
                           uint32_t                oid) :
    appearance(fb::game::appearance_factory::create(object)),
    message(std::string(message)),
    str1(std::string(str1)),
    str2(std::string(str2)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

void dialog_email::serialize(fb::stream_writer<big_endian>& writer) const
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
    writer.write<bool>(this->button_next);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<std::string, uint8_t>(this->str1);
    writer.write<uint8_t>(0x00); // unused pad (InputExt maxlen slot)
    writer.write<std::string, uint8_t>(this->str2);
}
#else
void dialog_email::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // first type echo
    this->type_echo = reader.read<uint8_t>();
    this->oid       = reader.read<uint32_t>();
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint8_t>();  // 0x01
    reader.read<uint16_t>(); // look
    reader.read<uint8_t>();  // color
    reader.read<uint8_t>();  // obj type flag
    reader.read<uint16_t>(); // look (duplicate)
    reader.read<uint8_t>();  // color (duplicate)
    reader.read<uint32_t>(); // 0x00000001
    this->button_prev = reader.read<bool>();
    this->button_next = reader.read<bool>();
    this->message     = reader.read<std::string, uint16_t>();
    this->str1        = reader.read<std::string, uint8_t>();
    reader.read<uint8_t>(); // unused pad
    this->str2 = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
