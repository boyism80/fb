#include <fb/game/protocol/item/info.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void item_info<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->position);
    // Real client WriteHeader chain leftovers; server ignores.
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(1);
    writer.write<uint8_t>(1);
    writer.write<uint8_t>(this->slot + 1);
}
#else
template <CLIENT_VERSION V>
void item_info<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->position = reader.read<uint16_t>();
    this->filler0  = reader.read<uint8_t>();
    this->filler1  = reader.read<uint8_t>();
    this->filler2  = reader.read<uint8_t>();
    this->slot     = reader.read<uint8_t>() - 1;
}
#endif

template class item_info<CLIENT_VERSION::v550>;
template class item_info<CLIENT_VERSION::v565>;
template class item_info<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
