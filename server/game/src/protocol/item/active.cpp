#include <fb/game/protocol/item/active.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void item_active<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index + 1);
}
#else
template <CLIENT_VERSION V>
void item_active<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
}
#endif

template class item_active<CLIENT_VERSION::v550>;
template class item_active<CLIENT_VERSION::v565>;
template class item_active<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
