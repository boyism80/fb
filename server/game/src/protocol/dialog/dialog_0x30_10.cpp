#include <fb/game/protocol/dialog/dialog_0x30_10.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
dialog_0x30_10<V>::dialog_0x30_10(const fb::model::object& obj,
                                  std::string_view         message,
                                  bool                     button_prev,
                                  bool                     button_next,
                                  uint32_t                 oid) :
    appearance(fb::game::appearance_factory::create<V>(obj)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_0x30_10<V>::dialog_0x30_10(const fb::game::object& object,
                                  std::string_view        message,
                                  bool                    button_prev,
                                  bool                    button_next,
                                  uint32_t                oid) :
    appearance(fb::game::appearance_factory::create<V>(object)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

template <CLIENT_VERSION V>
dialog_0x30_10<V>::dialog_0x30_10(appearance_ptr&& appearance,
                                  std::string_view message,
                                  bool             button_prev,
                                  bool             button_next,
                                  uint32_t         oid) :
    appearance(std::move(appearance)),
    message(std::string(message)),
    button_prev(button_prev),
    button_next(button_next),
    oid(oid)
{ }

template <CLIENT_VERSION V>
void dialog_0x30_10<V>::serialize(fb::stream_writer<big_endian>& writer) const
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

template <>
void dialog_0x30_10<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    constexpr auto type_value = static_cast<uint8_t>(type);

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(type_value);
    writer.write<uint8_t>(type_value);
    writer.write<uint32_t>(this->oid);
    this->appearance->serialize(writer);
    writer.write<uint32_t>(0);
    writer.write<uint32_t>(1);
    writer.write<bool>(this->button_prev);
    writer.write<bool>(this->button_next);
    writer.write<std::string, uint16_t>(this->message);
}

template class dialog_0x30_10<CLIENT_VERSION::v550>;
template class dialog_0x30_10<CLIENT_VERSION::v565>;
template class dialog_0x30_10<CLIENT_VERSION::v651>;
#else
void dialog_0x30_10::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    this->type_echo = reader.read<uint8_t>();
    this->oid       = reader.read<uint32_t>();
    reader.read<uint8_t>();
    reader.read<uint8_t>();
    reader.read<uint16_t>();
    reader.read<uint8_t>();
    reader.read<uint8_t>();
    reader.read<uint16_t>();
    reader.read<uint8_t>();
    reader.read<uint32_t>();
    this->button_prev = reader.read<bool>();
    this->button_next = reader.read<bool>();
    this->message     = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
