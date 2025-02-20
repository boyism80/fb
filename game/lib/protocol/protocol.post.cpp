#include <fb/game/protocol/post.h>

namespace fb::protocol::game::request {

async::task<void> post::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->slot = reader.read<uint8_t>();
}

} // namespace fb::protocol::game::request
