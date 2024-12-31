#include <fb/game/protocol/item/item_drop_cash.h>

namespace fb::protocol::game::request {

async::task<void> item_drop_cash::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->chunk = reader.read<uint32_t>();
}

} // namespace fb::protocol::game::request
