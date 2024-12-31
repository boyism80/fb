#include <fb/game/protocol/whisper.h>

namespace fb::protocol::game::request {

async::task<void> whisper::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->name    = reader.read<std::string, uint8_t>();
    this->message = reader.read<std::string, uint8_t>();
}

} // namespace fb::protocol::game::request