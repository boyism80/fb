#include <fb/game/protocol/emotion.h>

namespace fb::protocol::game::request {

#ifndef BOT // server only
template <CLIENT_VERSION V>
void emotion<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->value = reader.read<uint8_t>();
}
#else // bot only
template <CLIENT_VERSION V>
emotion<V>::emotion(uint8_t value) :
    value(value)
{ }

template <CLIENT_VERSION V>
void emotion<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->value);
}
#endif

template class emotion<CLIENT_VERSION::v550>;
template class emotion<CLIENT_VERSION::v565>;
template class emotion<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
