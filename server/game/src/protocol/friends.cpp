#include <fb/game/protocol/friends.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void friends<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
template <CLIENT_VERSION V>
void friends<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);

    auto count = reader.read<uint8_t>();
    for (int i = 0; i < count; i++)
    {
        auto name = reader.read<std::string, uint8_t>();
        this->names.push_back(name);
    }
}
#endif

template class friends<CLIENT_VERSION::v550>;
template class friends<CLIENT_VERSION::v565>;
template class friends<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
