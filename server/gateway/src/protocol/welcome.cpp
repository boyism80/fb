#include <fb/gateway/protocol/welcome.h>

namespace fb::protocol::gateway::response {

void welcome::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}

} // namespace fb::protocol::gateway::response
