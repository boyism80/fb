#include <fb/game/protocol/swap.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void swap<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<uint8_t>(this->src);
    writer.write<uint8_t>(this->dst);
}
#else
template <CLIENT_VERSION V>
void swap<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = SWAP_TYPE(reader.read<uint8_t>());
    this->src  = reader.read<uint8_t>();
    this->dst  = reader.read<uint8_t>();
}
#endif

template class swap<CLIENT_VERSION::v550>;
template class swap<CLIENT_VERSION::v565>;
template class swap<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
