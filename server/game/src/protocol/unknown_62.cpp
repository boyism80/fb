#include <fb/game/protocol/unknown_62.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
unknown_62<V>::unknown_62(uint8_t type, std::string url, std::string key, std::string cookie) :
    type(type),
    url(std::move(url)),
    key(std::move(key)),
    cookie(std::move(cookie))
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_62<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}
#else
template <CLIENT_VERSION V>
void unknown_62<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#endif

template class unknown_62<CLIENT_VERSION::v550>;
template class unknown_62<CLIENT_VERSION::v565>;
template class unknown_62<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
