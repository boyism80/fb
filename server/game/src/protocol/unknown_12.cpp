#include <fb/game/protocol/unknown_12.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
void unknown_12<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->slot);
    writer.write<uint8_t>(this->flag);
}
#else
template <CLIENT_VERSION V>
void unknown_12<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid  = reader.read<uint32_t>();
    this->slot = reader.read<uint8_t>();
    this->flag = reader.read<uint8_t>();
}
#endif

template class unknown_12<CLIENT_VERSION::v550>;
template class unknown_12<CLIENT_VERSION::v565>;
template class unknown_12<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
