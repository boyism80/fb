#include <fb/game/protocol/item/drop_money.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
item_drop_money<V>::item_drop_money(uint32_t chunk) :
    chunk(chunk)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void item_drop_money<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->chunk);
}
#else
template <CLIENT_VERSION V>
void item_drop_money<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->chunk = reader.read<uint32_t>();
}
#endif

template class item_drop_money<CLIENT_VERSION::v550>;
template class item_drop_money<CLIENT_VERSION::v565>;
template class item_drop_money<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
