#include <fb/game/protocol/pong.h>

namespace fb::protocol::game::request {

#ifndef BOT
template <CLIENT_VERSION V>
void pong<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->token          = reader.read<uint32_t>();
    this->client_tick_ms = reader.read<uint32_t>();
}
#else
template <CLIENT_VERSION V>
void pong<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->token);
    writer.write<uint32_t>(this->client_tick_ms);
}
#endif

template class pong<CLIENT_VERSION::v550>;
template class pong<CLIENT_VERSION::v565>;
template class pong<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
