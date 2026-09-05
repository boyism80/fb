#include <fb/game/protocol/item/tip.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
item_tip<V>::item_tip(uint16_t position, std::string_view message) :
    position(position),
    message(std::string(message))
{ }

template <CLIENT_VERSION V>
void item_tip<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->position);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}

template <>
void item_tip<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x40);
    writer.write<uint8_t>(0x00);
    writer.write<uint16_t>(1);
    writer.write<uint8_t>(1);
    writer.write<uint16_t>(this->position);
    writer.write<std::string, uint8_t>(this->message);
}
#else
template <CLIENT_VERSION V>
void item_tip<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->position = reader.read<uint16_t>();
    this->message  = reader.read<std::string, uint16_t>();
    reader.read<uint8_t>(); // 0x00
}

template <>
void item_tip<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    reader.read<uint8_t>();
    reader.read<uint16_t>();
    auto count = reader.read<uint8_t>();
    if (count > 0)
    {
        this->position = reader.read<uint16_t>();
        this->message  = reader.read<std::string, uint8_t>();
        for (uint8_t i = 1; i < count; ++i)
        {
            reader.read<uint16_t>();
            reader.read<std::string, uint8_t>();
        }
    }
}
#endif

template class item_tip<CLIENT_VERSION::v550>;
template class item_tip<CLIENT_VERSION::v565>;
template class item_tip<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
