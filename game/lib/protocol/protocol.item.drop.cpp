#include <fb/game/protocol/item/item_drop.h>

namespace fb::protocol::game::request {

async::task<void> item_drop::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
    this->all   = bool(reader.read<uint8_t>());
}

} // namespace fb::protocol::game::request
