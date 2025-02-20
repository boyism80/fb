#include <fb/game/protocol/give.h>

namespace fb::protocol::game::request {

async::task<void> give::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->slot = reader.read<uint8_t>();
    this->all  = reader.read<bool>();
}

} // namespace fb::protocol::game::request
