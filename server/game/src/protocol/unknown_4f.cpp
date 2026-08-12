#include <fb/game/protocol/unknown_4f.h>

namespace fb::protocol::game::response {

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_4f<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#else
template <CLIENT_VERSION V>
void unknown_4f<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#endif

template class unknown_4f<CLIENT_VERSION::v550>;
template class unknown_4f<CLIENT_VERSION::v565>;
template class unknown_4f<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
