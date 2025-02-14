#include <fb/game/protocol/object/miss.h>

namespace fb::protocol::game::request {

async::task<void> miss::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->sequence = reader.read<uint32_t>();
}

} // namespace fb::protocol::game::request
