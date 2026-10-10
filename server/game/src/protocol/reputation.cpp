#include <fb/game/protocol/reputation.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
reputation<V>::reputation(std::string_view name, bool raise) :
    name(std::string(name)),
    raise(raise)
{ }

template <CLIENT_VERSION V>
void reputation<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<uint8_t>(this->raise ? 1 : 0);
}
#else
template <CLIENT_VERSION V>
void reputation<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    this->name    = reader.read<std::string, uint8_t>();
    this->raise   = reader.read<uint8_t>() == 1;
}
#endif

template class reputation<CLIENT_VERSION::v550>;
template class reputation<CLIENT_VERSION::v565>;
template class reputation<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
