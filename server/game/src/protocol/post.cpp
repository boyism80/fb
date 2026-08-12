#include <fb/game/protocol/post.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void post<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->slot);
}
#else
template <CLIENT_VERSION V>
void post<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->slot = reader.read<uint8_t>();
}
#endif

template class post<CLIENT_VERSION::v550>;
template class post<CLIENT_VERSION::v565>;
template class post<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
