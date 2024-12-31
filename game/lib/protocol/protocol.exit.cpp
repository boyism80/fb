#include <fb/game/protocol/exit.h>

namespace fb::protocol::game::request {

async::task<void> exit::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}
} // namespace fb::protocol::game::request
