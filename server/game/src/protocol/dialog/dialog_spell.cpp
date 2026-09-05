#include <fb/game/protocol/dialog/dialog_spell.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
dialog_spell<V>::dialog_spell(const fb::model::object& obj, std::string_view message, uint32_t oid, uint16_t pursuit) :
    appearance(fb::game::appearance_factory::create<V>(obj)),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_spell<V>::dialog_spell(const fb::game::object& object,
                              std::string_view        message,
                              uint32_t                oid,
                              uint16_t                pursuit) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_spell<V>::dialog_spell(appearance_ptr&& appearance, std::string_view message, uint32_t oid, uint16_t pursuit) :
    appearance(std::move(appearance)),
    message(std::string(message)),
    pursuit(pursuit),
    oid(oid)
{ }

template <CLIENT_VERSION V>
void dialog_spell<V>::serialize(fb::stream_writer<big_endian>& writer) const
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

template <>
void dialog_spell<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(0);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint16_t>(this->pursuit);
}

template class dialog_spell<CLIENT_VERSION::v550>;
template class dialog_spell<CLIENT_VERSION::v565>;
template class dialog_spell<CLIENT_VERSION::v651>;
#else
void dialog_spell::deserialize(fb::stream_reader<big_endian>& reader)
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
    this->message = reader.read<std::string, uint16_t>();
    this->pursuit = reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
