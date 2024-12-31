#include <fb/game/protocol/update_screen.h>

namespace fb::protocol::game::request {

async::task<void> update_screen::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}

} // namespace fb::protocol::game::request
