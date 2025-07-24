#include <fb/game/protocol/whisper.h>

namespace fb::protocol::game::request {

#ifdef BOT // bot only
async::task<void> whisper::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<std::string, uint8_t>(this->message);
}
#else // server only
async::task<void> whisper::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->name    = reader.read<std::string, uint8_t>();
    this->message = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::request