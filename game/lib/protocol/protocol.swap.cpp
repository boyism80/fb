#include <fb/game/protocol/swap.h>

namespace fb::protocol::game::request {

async::task<void> swap::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->type = SWAP_TYPE(reader.read<uint8_t>());
    this->src  = reader.read<uint8_t>();
    this->dst  = reader.read<uint8_t>();
}

} // namespace fb::protocol::game::request
