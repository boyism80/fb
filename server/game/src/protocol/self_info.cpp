#include <fb/game/protocol/self_info.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void self_info<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
template <CLIENT_VERSION V>
void self_info<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}

template <>
void self_info<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    if (reader.readable_size() > 0)
        reader.read<uint8_t>();
}
#endif

template class self_info<CLIENT_VERSION::v550>;
template class self_info<CLIENT_VERSION::v565>;
template class self_info<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
