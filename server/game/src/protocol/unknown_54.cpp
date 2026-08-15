#include <fb/game/protocol/unknown_54.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
unknown_54<V>::unknown_54(uint32_t token, uint8_t phase) :
    token(token),
    phase(phase)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_54<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x01);
    writer.write<uint32_t>(this->token);
    writer.write<uint8_t>(this->phase);
}
#else
template <CLIENT_VERSION V>
void unknown_54<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // constant 0x01
    this->token = reader.read<uint32_t>();
    this->phase = reader.read<uint8_t>();
}

template <>
void unknown_54<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // constant 0x01
    this->token = reader.read<uint32_t>();
    this->phase = reader.read<uint8_t>();
    if (this->phase == 0)
    {
        if (reader.readable_size() >= 5)
        {
            reader.read<uint8_t>();
            reader.read<uint32_t>();
        }
    }
    else if (this->phase == 1)
    {
        if (reader.readable_size() >= 12)
        {
            reader.read<uint32_t>();
            reader.read<uint32_t>();
            reader.read<uint16_t>();
            reader.read<uint16_t>();
        }
    }
    else if (this->phase == 2)
    {
        if (reader.readable_size() >= 12)
        {
            reader.read<uint32_t>();
            reader.read<uint32_t>();
            reader.read<uint32_t>();
        }
    }
    else if (this->phase == 3)
    {
        if (reader.readable_size() >= 4)
            reader.read<uint32_t>();
    }
}
#endif

template class unknown_54<CLIENT_VERSION::v550>;
template class unknown_54<CLIENT_VERSION::v565>;
template class unknown_54<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
