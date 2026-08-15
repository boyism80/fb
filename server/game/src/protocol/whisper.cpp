#include <fb/game/protocol/whisper.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
whisper<V>::whisper(std::string_view name, std::string_view message) :
    name(std::string(name)),
    message(std::string(message))
{ }
#endif

#ifdef BOT // bot only
template <CLIENT_VERSION V>
void whisper<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<std::string, uint8_t>(this->message);
}
#else // server only
template <CLIENT_VERSION V>
void whisper<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->name    = reader.read<std::string, uint8_t>();
    this->message = reader.read<std::string, uint8_t>();
}
#endif

template class whisper<CLIENT_VERSION::v550>;
template class whisper<CLIENT_VERSION::v565>;
template class whisper<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request