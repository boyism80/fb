#include <fb/game/protocol/item/drop.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
item_drop<V>::item_drop(uint8_t index, bool all) :
    index(index),
    all(all)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void item_drop<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index);
    writer.write<bool>(this->all);
}
#else
template <CLIENT_VERSION V>
void item_drop<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
    this->all   = bool(reader.read<uint8_t>());
}
#endif

template class item_drop<CLIENT_VERSION::v550>;
template class item_drop<CLIENT_VERSION::v565>;
template class item_drop<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
