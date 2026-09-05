#include <fb/game/protocol/update_move.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void update_move<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    move<V>::serialize(writer);
    writer.write<uint16_t>(this->begin.x);
    writer.write<uint16_t>(this->begin.y);
    writer.write<uint8_t>(this->size.width);
    writer.write<uint8_t>(this->size.height);
    writer.write<uint16_t>(this->crc);
}
#else
template <CLIENT_VERSION V>
void update_move<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    move<V>::deserialize(reader);

    this->begin.x     = reader.read<uint16_t>();
    this->begin.y     = reader.read<uint16_t>();
    this->size.width  = reader.read<uint8_t>();
    this->size.height = reader.read<uint8_t>();
    this->crc         = reader.read<uint16_t>();
}
#endif

template class update_move<CLIENT_VERSION::v550>;
template class update_move<CLIENT_VERSION::v565>;
template class update_move<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
