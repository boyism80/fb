#include <fb/game/protocol/click.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void click<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->flag);
    writer.write<uint32_t>(this->oid);
}
#else
template <CLIENT_VERSION V>
void click<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->flag = reader.read<uint8_t>();
    this->oid  = reader.read<uint32_t>();
}
#endif

template class click<CLIENT_VERSION::v550>;
template class click<CLIENT_VERSION::v565>;
template class click<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
