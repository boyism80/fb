#include <fb/game/protocol/dialog/dialog_spell.h>

namespace fb::protocol::game::response {

#ifndef BOT
dialog_spell::dialog_spell(const fb::model::object& obj, std::string_view message, uint32_t oid, uint16_t pursuit) :
    appearance(fb::game::appearance_factory::create(obj)),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

dialog_spell::dialog_spell(const fb::game::object& object, std::string_view message, uint32_t oid, uint16_t pursuit) :
    appearance(fb::game::appearance_factory::create(object)),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

void dialog_spell::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(this->pursuit);
}
#endif

} // namespace fb::protocol::game::response
