#include <fb/game/protocol/item/item_inactive.h>

namespace fb::protocol::game::request {

[[nodiscard]] async::task<void> item_inactive::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->parts = EQUIPMENT_PARTS(reader.read<uint8_t>());
}

} // namespace fb::protocol::game::request
