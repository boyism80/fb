#include <fb/game/protocol/self_info.h>

namespace fb::protocol::game::request {

async::task<void> self_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}

} // namespace fb::protocol::game::request
