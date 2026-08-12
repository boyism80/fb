#include <fb/game/protocol/item/throws.h>

namespace fb::protocol::game::request {
#ifdef BOT
template <CLIENT_VERSION V>
void item_throws<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<bool>(this->all);
    writer.write<uint8_t>(this->index + 1);
}
#else
template <CLIENT_VERSION V>
void item_throws<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->all   = reader.read<bool>();
    this->index = reader.read<uint8_t>() - 1;
}
#endif

template class item_throws<CLIENT_VERSION::v550>;
template class item_throws<CLIENT_VERSION::v565>;
template class item_throws<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

#ifndef BOT
item_throws::item_throws(const fb::game::character&        ch,
                         const fb::game::item&             item,
                         const fb::model::point<uint16_t>& to) :
    ch(ch),
    item(item),
    to(to)
{ }
#endif

#ifndef BOT
void item_throws::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->ch.oid());
    writer.write<uint16_t>(this->item.look());
    writer.write<uint8_t>(this->item.color());
    writer.write<uint32_t>(this->item.oid());
    writer.write<uint16_t>(this->ch.x());
    writer.write<uint16_t>(this->ch.y());
    writer.write<uint16_t>(this->to.x);
    writer.write<uint16_t>(this->to.y);
    writer.write<uint32_t>(0); // projectile legacy slot (client stores, never reads)
    writer.write<uint8_t>(static_cast<uint8_t>(ACTION::THROW));
}
#else
void item_throws::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->ch_oid   = reader.read<uint32_t>();
    this->look     = reader.read<uint16_t>();
    this->color    = reader.read<uint8_t>();
    this->item_oid = reader.read<uint32_t>();
    this->from_x   = reader.read<uint16_t>();
    this->from_y   = reader.read<uint16_t>();
    this->to_x     = reader.read<uint16_t>();
    this->to_y     = reader.read<uint16_t>();
    reader.read<uint32_t>(); // projectile legacy slot
    reader.read<uint8_t>();  // ACTION::THROW
}
#endif

} // namespace fb::protocol::game::response