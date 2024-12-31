#include <fb/game/protocol/door.h>

namespace fb::protocol::game::request {

async::task<void> door::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}

} // namespace fb::protocol::game::request
