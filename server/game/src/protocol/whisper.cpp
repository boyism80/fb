#include <fb/game/protocol/whisper.h>

namespace fb::protocol::game::request {

#ifdef BOT // bot only
void whisper::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<std::string, uint8_t>(this->message);
}
#else // server only
void whisper::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->name    = reader.read<std::string, uint8_t>();
    this->message = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::request