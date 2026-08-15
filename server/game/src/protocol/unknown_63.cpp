#include <fb/game/protocol/unknown_63.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
unknown_63<V>::unknown_63(uint8_t subtype, uint8_t count, std::vector<unknown_63_entry> entries) :
    subtype(subtype),
    count(count),
    entries(std::move(entries))
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_63<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    this->count   = reader.read<uint8_t>();
}
#else
template <CLIENT_VERSION V>
void unknown_63<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    writer.write<uint8_t>(this->count);
}
#endif

template class unknown_63<CLIENT_VERSION::v550>;
template class unknown_63<CLIENT_VERSION::v565>;
template class unknown_63<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
