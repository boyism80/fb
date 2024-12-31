#include <fb/game/protocol/item/item_mix.h>

namespace fb::protocol::game::request {

async::task<void> item_mix::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    auto count = reader.read<uint8_t>();
    for (int i = 0; i < count; i++)
        this->indices.push_back(reader.read<uint8_t>());
}

} // namespace fb::protocol::game::request
