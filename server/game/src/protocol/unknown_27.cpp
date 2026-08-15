#include <fb/game/protocol/unknown_27.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
unknown_27<V>::unknown_27(uint8_t slot) :
    slot(slot)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_27<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(this->slot);
}
#else
template <CLIENT_VERSION V>
void unknown_27<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // constant 0x00
    reader.read<uint8_t>(); // constant 0x01
    this->slot = reader.read<uint8_t>();
}
#endif

template class unknown_27<CLIENT_VERSION::v550>;
template class unknown_27<CLIENT_VERSION::v565>;
template class unknown_27<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
