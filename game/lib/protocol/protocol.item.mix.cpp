#include <fb/game/protocol/item/item_mix.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> item_mix::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->indices.size()));
    for (const auto& index : this->indices)
    {
        writer.write<uint8_t>(index);
    }
}
#else
async::task<void> item_mix::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    auto count = reader.read<uint8_t>();
    for (int i = 0; i < count; i++)
        this->indices.push_back(reader.read<uint8_t>());
}
#endif
} // namespace fb::protocol::game::request
