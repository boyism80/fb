#include <fb/game/protocol/loot.h>

namespace fb::protocol::game::request {

#ifndef BOT
template <CLIENT_VERSION V>
void loot<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->boost = bool(reader.read<uint8_t>());
}
#else
template <CLIENT_VERSION V>
loot<V>::loot(bool boost) :
    boost(boost)
{ }

template <CLIENT_VERSION V>
void loot<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->boost);
}
#endif

template class loot<CLIENT_VERSION::v550>;
template class loot<CLIENT_VERSION::v565>;
template class loot<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
