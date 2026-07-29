#include <fb/game/protocol/dialog/input_ext.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_input_ext::dialog_input_ext(const fb::model::object&   obj,
                                   std::optional<std::string> message,
                                   std::string_view           top,
                                   std::string_view           bottom,
                                   int                        maxlen,
                                   bool                       button_prev,
                                   bool                       password,
                                   uint32_t                   oid) :
    appearance(fb::game::appearance_factory::create(obj)),
    message(std::move(message)),
    top(std::string(top)),
    bottom(std::string(bottom)),
    maxlen(maxlen),
    button_prev(button_prev),
    password(password),
    oid(oid)
{ }

dialog_input_ext::dialog_input_ext(const fb::game::object&    object,
                                   std::optional<std::string> message,
                                   std::string_view           top,
                                   std::string_view           bottom,
                                   int                        maxlen,
                                   bool                       button_prev,
                                   bool                       password,
                                   uint32_t                   oid) :
    appearance(fb::game::appearance_factory::create(object)),
    message(std::move(message)),
    top(std::string(top)),
    bottom(std::string(bottom)),
    maxlen(maxlen),
    button_prev(button_prev),
    password(password),
    oid(oid)
{ }

void dialog_input_ext::serialize(fb::stream_writer<big_endian>& writer) const
{
    // message + password → subtype 4/5/7/8
    uint8_t type_value;
    if (this->password)
    {
        type_value = static_cast<uint8_t>(this->message.has_value() ? fb::game::dialog::list_type::INPUT_PASSWORD
                                                                    : fb::game::dialog::list_type::INPUT_PASSWORD_NO_MSG);
    }
    else
    {
        type_value = static_cast<uint8_t>(this->message.has_value() ? fb::game::dialog::list_type::INPUT
                                                                    : fb::game::dialog::list_type::INPUT_NO_MSG);
    }

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(1);
    writer.write<bool>(this->button_prev);
    writer.write<uint8_t>(0x00); // flag_next (client reads, unused for InputExt)
    if (this->message.has_value())
        writer.write<std::string, uint16_t>(*this->message);
    writer.write<std::string, uint8_t>(this->top);
    writer.write<uint8_t>(this->maxlen);
    writer.write<std::string, uint8_t>(this->bottom);
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
    reader.read<uint8_t>(); // flag_next

    this->password = (this->type_echo == static_cast<uint8_t>(fb::game::dialog::list_type::INPUT_PASSWORD) ||
                      this->type_echo == static_cast<uint8_t>(fb::game::dialog::list_type::INPUT_PASSWORD_NO_MSG));

    this->message = std::nullopt;
    if (this->type_echo == static_cast<uint8_t>(fb::game::dialog::list_type::INPUT) ||
        this->type_echo == static_cast<uint8_t>(fb::game::dialog::list_type::INPUT_PASSWORD))
        this->message = reader.read<std::string, uint16_t>();

    this->top    = reader.read<std::string, uint8_t>();
    this->maxlen = reader.read<uint8_t>();
    this->bottom = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
