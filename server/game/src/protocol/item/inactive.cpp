#include <fb/game/protocol/item/inactive.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
item_inactive<V>::item_inactive(EQUIPMENT_PARTS parts) :
    parts(parts)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void item_inactive<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
}
#else
template <CLIENT_VERSION V>
void item_inactive<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->parts = static_cast<EQUIPMENT_PARTS>(reader.read<uint8_t>());
}
#endif

template class item_inactive<CLIENT_VERSION::v550>;
template class item_inactive<CLIENT_VERSION::v565>;
template class item_inactive<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
