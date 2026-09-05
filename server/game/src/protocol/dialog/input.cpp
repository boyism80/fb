#include <fb/game/protocol/dialog/input.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
dialog_input<V>::dialog_input(const fb::model::object&   obj,
                              std::string_view           message,
                              uint32_t                   oid,
                              std::optional<std::string> ext,
                              uint16_t                   pursuit) :
    appearance(fb::game::appearance_factory::create<V>(obj)),
    message(std::string(message)),
    oid(oid),
    ext(std::move(ext)),
    pursuit(pursuit)
{ }

template <CLIENT_VERSION V>
dialog_input<V>::dialog_input(const fb::game::object&    object,
                              std::string_view           message,
                              uint32_t                   oid,
                              std::optional<std::string> ext,
                              uint16_t                   pursuit) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    message(std::string(message)),
    oid(oid),
    ext(std::move(ext)),
    pursuit(pursuit)
{ }

template <CLIENT_VERSION V>
dialog_input<V>::dialog_input(appearance_ptr&&           appearance,
                              std::string_view           message,
                              uint32_t                   oid,
                              std::optional<std::string> ext,
                              uint16_t                   pursuit) :
    appearance(std::move(appearance)),
    message(std::string(message)),
    oid(oid),
    ext(std::move(ext)),
    pursuit(pursuit)
{ }

template <CLIENT_VERSION V>
void dialog_input<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto type_value = static_cast<uint8_t>(this->ext.has_value() ? fb::game::dialog::type::INPUT
                                                                 : fb::game::dialog::type::INPUT_NO_EXT);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<std::string, uint16_t>(this->message);
    if (this->ext.has_value())
        writer.write<std::string, uint8_t>(*this->ext);
    writer.write<uint16_t>(this->pursuit);
}

template <>
void dialog_input<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto type_value = static_cast<uint8_t>(this->ext.has_value() ? fb::game::dialog::type::INPUT
                                                                 : fb::game::dialog::type::INPUT_NO_EXT);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(0);
    writer.write<std::string, uint16_t>(this->message);
    if (this->ext.has_value())
        writer.write<std::string, uint8_t>(*this->ext);
    writer.write<uint16_t>(this->pursuit);
}

template class dialog_input<CLIENT_VERSION::v550>;
template class dialog_input<CLIENT_VERSION::v565>;
template class dialog_input<CLIENT_VERSION::v651>;
#else
void dialog_input::deserialize(fb::stream_reader<big_endian>& reader)
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
    this->message = reader.read<std::string, uint16_t>();

    this->ext = std::nullopt;
    if (this->type_echo == static_cast<uint8_t>(fb::game::dialog::type::INPUT))
        this->ext = reader.read<std::string, uint8_t>();

    this->pursuit = reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
