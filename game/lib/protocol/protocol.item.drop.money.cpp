#include <fb/game/protocol/item/item_drop_money.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> item_drop_money::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->chunk);
}
#else
async::task<void> item_drop_money::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->chunk = reader.read<uint32_t>();
}
#endif
} // namespace fb::protocol::game::request
