#include <fb/game/protocol/door.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void door<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
template <CLIENT_VERSION V>
void door<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif

template class door<CLIENT_VERSION::v550>;
template class door<CLIENT_VERSION::v565>;
template class door<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
