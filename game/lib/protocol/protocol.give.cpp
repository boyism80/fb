#include <fb/game/protocol/give.h>

namespace fb::protocol::game::request {

async::task<void> give_item::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->slot = reader.read<uint8_t>();
    this->all  = reader.read<bool>();
}

async::task<void> give_money::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->money = reader.read<uint32_t>();
}

} // namespace fb::protocol::game::request
