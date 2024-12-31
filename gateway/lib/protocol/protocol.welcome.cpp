#include <fb/gateway/protocol/welcome.h>

namespace fb::protocol::gateway::response {

async::task<void> welcome::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}

} // namespace fb::protocol::gateway::response
