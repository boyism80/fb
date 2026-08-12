#include <fb/game/protocol/attack.h>

namespace fb::protocol::game::request {

#ifndef BOT
template <CLIENT_VERSION V>
void attack<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#else
template <CLIENT_VERSION V>
void attack<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#endif

template class attack<CLIENT_VERSION::v550>;
template class attack<CLIENT_VERSION::v565>;
template class attack<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
