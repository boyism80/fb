#include <fb/game/protocol/object/effect.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
effect<V>::effect(const fb::game::object& me, uint8_t value) :
    me(me),
    value(value)
{ }
#endif

#ifndef BOT
template <CLIENT_VERSION V>
void effect<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->me.oid());
    if constexpr (V == CLIENT_VERSION::v550)
    {
        writer.write<uint8_t>(this->value);
        writer.write<uint8_t>(0x00);
    }
    else if constexpr (V == CLIENT_VERSION::v565)
    {
        writer.write<uint8_t>(this->value);
        writer.write<uint16_t>(0); // duration
        if (this->value != 0x86)
        {
            writer.write<uint16_t>(0); // x
            writer.write<uint16_t>(0); // y
        }
    }
    else // CLIENT_VERSION::v651
    {
        writer.write<uint16_t>(this->value);
        writer.write<uint16_t>(0); // duration
        if (this->value != 0x86)
        {
            writer.write<uint16_t>(0); // x
            writer.write<uint16_t>(0); // y
        }
    }
}
#else
template <CLIENT_VERSION V>
void effect<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid = reader.read<uint32_t>();
    if constexpr (V == CLIENT_VERSION::v550)
    {
        this->value = reader.read<uint8_t>();
        reader.read<uint8_t>(); // 0x00
    }
    else if constexpr (V == CLIENT_VERSION::v565)
    {
        this->value = reader.read<uint8_t>();
        reader.read<uint16_t>(); // duration
        if (this->value != 0x86)
        {
            reader.read<uint16_t>(); // x
            reader.read<uint16_t>(); // y
        }
    }
    else // CLIENT_VERSION::v651
    {
        this->value = static_cast<uint8_t>(reader.read<uint16_t>());
        reader.read<uint16_t>(); // duration
        if (this->value != 0x86)
        {
            reader.read<uint16_t>(); // x
            reader.read<uint16_t>(); // y
        }
    }
}
#endif

template class effect<CLIENT_VERSION::v550>;
template class effect<CLIENT_VERSION::v565>;
template class effect<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
