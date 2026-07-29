#include <fb/game/protocol/dialog/dialog_0x30_10.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_0x30_10::dialog_0x30_10(const fb::model::object& obj,
                               std::string_view         message,
                               bool                     button_prev,
                               bool                     button_next,
                               uint32_t                 oid) :
    appearance(fb::game::appearance_factory::create(obj)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

dialog_0x30_10::dialog_0x30_10(const fb::game::object& object,
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

void dialog_0x30_10::serialize(fb::stream_writer<big_endian>& writer) const
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
}
#endif

} // namespace fb::protocol::game::response
