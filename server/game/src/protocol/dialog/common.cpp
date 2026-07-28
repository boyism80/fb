#include <fb/game/protocol/dialog.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog::dialog(std::string_view message, bool button_prev, bool button_next, uint32_t oid) :
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

dialog::dialog(const fb::model::object& object,
               std::string_view         message,
               bool                     button_prev,
               bool                     button_next,
               uint32_t                 oid) :
    appearance(fb::game::appearance_factory::create(object)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

dialog::dialog(const fb::game::object& object,
               std::string_view        message,
               bool                    button_prev,
               bool                    button_next,
               uint32_t                oid) :
    appearance(fb::game::appearance_factory::create(object)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }
#endif

#ifndef BOT
void dialog::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    if (this->appearance != nullptr)
    {
        this->appearance->serialize(writer);
    }
    else
    {
        writer.write<uint8_t>(0x00);  // empty look flags
        writer.write<uint8_t>(0x01);  // empty look sex
        writer.write<uint16_t>(0x00); // empty look hair
        writer.write<uint8_t>(0x00);  // empty look color
        writer.write<uint8_t>(0x00);  // empty look flags (dup)
        writer.write<uint16_t>(0x00); // empty look hair (dup)
        writer.write<uint8_t>(0x00);  // empty look color (dup)
    }
    writer.write<uint32_t>(0x01); // seq seed echoed by client
    writer.write<bool>(this->button_prev);
    writer.write<bool>(this->button_next);
    writer.write<std::string, uint16_t>(this->message);
}
#else
void dialog::deserialize(fb::stream_reader<big_endian>& reader)
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
    reader.read<uint32_t>(); // 0x01
    this->button_prev = reader.read<bool>();
    this->button_next = reader.read<bool>();
    this->message     = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
