#include <fb/game/protocol/item/item_info.h>

namespace fb::protocol::game::request {

async::task<void> item_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->position = reader.read<uint16_t>();
    auto unknown1  = reader.read<uint8_t>();
    auto unknown2  = reader.read<uint8_t>();
    auto unknown3  = reader.read<uint8_t>();
    this->slot     = reader.read<uint8_t>() - 1;
}

} // namespace fb::protocol::game::request
