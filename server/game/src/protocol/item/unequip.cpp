#include <fb/game/protocol/item/unequip.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
item_unequip<V>::item_unequip(EQUIPMENT_PARTS parts) :
    parts(parts)
{ }
#endif

#ifndef BOT
template <CLIENT_VERSION V>
void item_unequip<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
    writer.write<uint8_t>(0x00);
}

template <>
void item_unequip<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
}
#else
template <CLIENT_VERSION V>
void item_unequip<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->parts = static_cast<EQUIPMENT_PARTS>(reader.read<uint8_t>());
    reader.read<uint8_t>();
}
#endif

template class item_unequip<CLIENT_VERSION::v550>;
template class item_unequip<CLIENT_VERSION::v565>;
template class item_unequip<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
