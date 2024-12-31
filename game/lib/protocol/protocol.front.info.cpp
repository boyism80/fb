#include <fb/game/protocol/front_info.h>

namespace fb::protocol::game::request {

async::task<void> front_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}

} // namespace fb::protocol::game::request
