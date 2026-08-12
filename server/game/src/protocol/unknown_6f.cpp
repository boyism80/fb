#include <fb/game/protocol/unknown_6f.h>

namespace fb::protocol::game::response {

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_6f<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    this->count   = reader.read<uint16_t>();
}
#else
template <CLIENT_VERSION V>
void unknown_6f<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    writer.write<uint16_t>(this->count);
}
#endif

template class unknown_6f<CLIENT_VERSION::v550>;
template class unknown_6f<CLIENT_VERSION::v565>;
template class unknown_6f<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
