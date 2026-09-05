#include <fb/game/protocol/give.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
give_item<V>::give_item(uint8_t slot, bool all) :
    slot(slot),
    all(all)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
give_money<V>::give_money(uint32_t money) :
    money(money)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void give_item<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->slot + 1);
    writer.write<bool>(this->all);
}
#else
template <CLIENT_VERSION V>
void give_item<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->slot = reader.read<uint8_t>() - 1;
    this->all  = reader.read<bool>();
}
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void give_money<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->money);
}
#else
template <CLIENT_VERSION V>
void give_money<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->money = reader.read<uint32_t>();
}
#endif

template class give_item<CLIENT_VERSION::v550>;
template class give_item<CLIENT_VERSION::v565>;
template class give_item<CLIENT_VERSION::v651>;
template class give_money<CLIENT_VERSION::v550>;
template class give_money<CLIENT_VERSION::v565>;
template class give_money<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
