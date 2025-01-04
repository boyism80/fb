#include <fb/game/protocol/item/item_active.h>

namespace fb::protocol::game::request {

async::task<void> item_active::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
}

} // namespace fb::protocol::game::request
