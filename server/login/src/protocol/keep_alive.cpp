#include <fb/login/protocol/keep_alive.h>

#include <cstdint>

namespace fb::protocol::login::request {

#ifdef BOT
template <CLIENT_VERSION V>
void keep_alive<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
template <CLIENT_VERSION V>
void keep_alive<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#endif

template class keep_alive<CLIENT_VERSION::v550>;
template class keep_alive<CLIENT_VERSION::v565>;
template class keep_alive<CLIENT_VERSION::v651>;

} // namespace fb::protocol::login::request
