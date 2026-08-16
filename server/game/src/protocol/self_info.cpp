#include <fb/game/protocol/self_info.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void self_info<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}

self_info<CLIENT_VERSION::v651>::self_info(bool party) :
    party(party)
{ }

void self_info<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->party ? 1 : 0);
}
#else
template <CLIENT_VERSION V>
void self_info<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}

void self_info<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->party = false;
    if (reader.readable_size() > 0)
        this->party = reader.read<uint8_t>() != 0;
}
#endif

template class self_info<CLIENT_VERSION::v550>;
template class self_info<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::request
