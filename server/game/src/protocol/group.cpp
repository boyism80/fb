#include <fb/game/protocol/group.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
group<V>::group(std::string_view name) :
    name(std::string(name))
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void group<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->name);
}
#else
template <CLIENT_VERSION V>
void group<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->name = reader.read<std::string, uint8_t>();
}
#endif

template class group<CLIENT_VERSION::v550>;
template class group<CLIENT_VERSION::v565>;
template class group<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
