#include <fb/game/protocol/web.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
web<V>::web(uint8_t type, std::string address, std::string message) :
    type(type),
    address(address),
    message(message)
{ }

template <>
void web<CLIENT_VERSION::v550>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->type);
    writer.write<std::string, uint16_t>(this->address);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}

template <>
void web<CLIENT_VERSION::v565>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(3);
    writer.write<std::string, uint16_t>(this->address);
    writer.write<std::string, uint16_t>(this->message);
    writer.write<uint8_t>(0x00);
}

template <>
void web<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->type);
    if (this->type == 0)
    {
        writer.write<std::string, uint16_t>(this->address);
    }
    else if (this->type == 1 || this->type == 2)
    {
        writer.write<std::string, uint16_t>(this->address);
        writer.write<std::string, uint16_t>(this->message);
    }
    else if (this->type == 3)
    {
        writer.write<std::string, uint8_t>(this->address);
    }
}
#else
template <CLIENT_VERSION V>
void web<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type    = reader.read<uint8_t>();
    this->address = reader.read<std::string, uint16_t>();
    this->message = reader.read<std::string, uint16_t>();
    reader.read<uint8_t>(); // 0x00
}

template <>
void web<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = reader.read<uint8_t>();
    if (this->type == 0)
    {
        this->address = reader.read<std::string, uint16_t>();
    }
    else if (this->type == 1 || this->type == 2)
    {
        this->address = reader.read<std::string, uint16_t>();
        this->message = reader.read<std::string, uint16_t>();
    }
    else if (this->type == 3)
    {
        this->address = reader.read<std::string, uint8_t>();
    }
}
#endif

template class web<CLIENT_VERSION::v550>;
template class web<CLIENT_VERSION::v565>;
template class web<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
