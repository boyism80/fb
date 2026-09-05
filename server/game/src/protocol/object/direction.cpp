#include <fb/game/protocol/object/direction.h>

namespace fb::protocol::game::request {

#ifndef BOT
template <CLIENT_VERSION V>
void direction<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->value = DIRECTION(reader.read<uint8_t>());
}
#else
template <CLIENT_VERSION V>
direction<V>::direction(DIRECTION value) :
    value(value)
{ }

template <CLIENT_VERSION V>
void direction<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>((uint8_t)this->value);
}
#endif

template class direction<CLIENT_VERSION::v550>;
template class direction<CLIENT_VERSION::v565>;
template class direction<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
direction::direction(const fb::game::object& object) :
    direction(object.oid(), object.direction())
{ }
direction::direction(uint32_t oid, DIRECTION value) :
    oid(oid),
    value(value)
{ }
#endif

#ifndef BOT
void direction::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(static_cast<uint8_t>(this->value));
    writer.write<uint8_t>(0x00);
}
#else
void direction::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid   = reader.read<uint32_t>();
    this->value = (DIRECTION)reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response