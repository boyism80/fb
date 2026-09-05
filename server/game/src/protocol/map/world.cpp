#include <fb/game/protocol/map/world.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
map_world<V>::map_world(uint16_t value, uint16_t before, uint16_t after) :
    value(value),
    before(before),
    after(after)
{ }
#endif

#ifndef BOT // server only
template <CLIENT_VERSION V>
void map_world<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->value  = reader.read<uint16_t>();
    this->before = reader.read<uint16_t>();
    this->after  = reader.read<uint16_t>();
}
#else // bot only
template <CLIENT_VERSION V>
void map_world<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->value);
    writer.write<uint16_t>(this->before);
    writer.write<uint16_t>(this->after);
}
#endif

template class map_world<CLIENT_VERSION::v550>;
template class map_world<CLIENT_VERSION::v565>;
template class map_world<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
