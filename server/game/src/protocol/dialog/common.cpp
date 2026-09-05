#include <fb/game/protocol/dialog.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
dialog<V>::dialog(std::optional<std::string> message, bool button_prev, bool button_next, uint32_t oid) :
    message(std::move(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog<V>::dialog(const fb::model::object&   object,
                  std::optional<std::string> message,
                  bool                       button_prev,
                  bool                       button_next,
                  uint32_t                   oid) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    message(std::move(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog<V>::dialog(const fb::game::object&    object,
                  std::optional<std::string> message,
                  bool                       button_prev,
                  bool                       button_next,
                  uint32_t                   oid) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    message(std::move(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog<V>::dialog(appearance_ptr&&           appearance,
                  std::optional<std::string> message,
                  bool                       button_prev,
                  bool                       button_next,
                  uint32_t                   oid) :
    appearance(std::move(appearance)),
    message(std::move(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

template <CLIENT_VERSION V>
void dialog<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto type_value = static_cast<uint8_t>(this->message.has_value() ? fb::game::dialog::list_type::TEXT
                                                                     : fb::game::dialog::list_type::TEXT_NO_MSG);

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
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x00);
        writer.write<uint16_t>(0x00);
        writer.write<uint8_t>(0x00);
    }
    writer.write<uint32_t>(0x01);
    writer.write<bool>(this->button_prev);
    writer.write<bool>(this->button_next);
    if (this->message.has_value())
        writer.write<std::string, uint16_t>(*this->message);
}

template <>
void dialog<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto type_value = static_cast<uint8_t>(this->message.has_value() ? fb::game::dialog::list_type::TEXT
                                                                     : fb::game::dialog::list_type::TEXT_NO_MSG);

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
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(0x00);
        writer.write<uint8_t>(0x00);
    }
    writer.write<uint32_t>(0);
    writer.write<uint32_t>(0x01);
    writer.write<bool>(this->button_prev);
    writer.write<bool>(this->button_next);
    if (this->message.has_value())
        writer.write<std::string, uint16_t>(*this->message);
}

template class dialog<CLIENT_VERSION::v550>;
template class dialog<CLIENT_VERSION::v565>;
template class dialog<CLIENT_VERSION::v651>;
#else
void dialog::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    this->type_echo = reader.read<uint8_t>();
    this->oid       = reader.read<uint32_t>();
    reader.read<uint8_t>();
    reader.read<uint8_t>();
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    reader.read<uint8_t>();
    reader.read<uint16_t>();
    reader.read<uint8_t>();
    reader.read<uint32_t>();
    this->button_prev = reader.read<bool>();
    this->button_next = reader.read<bool>();
    this->message     = std::nullopt;
    if (this->type_echo == static_cast<uint8_t>(fb::game::dialog::list_type::TEXT))
        this->message = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
