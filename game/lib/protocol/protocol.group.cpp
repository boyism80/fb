#include <fb/game/protocol/group.h>

namespace fb::protocol::game::request {

async::task<void> group::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->name = reader.read<std::string, uint8_t>();
}

} // namespace fb::protocol::game::request
