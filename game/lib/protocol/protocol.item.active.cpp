#include <fb/game/protocol/item/item_active.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> item_active::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->index + 1);
}
#else
async::task<void> item_active::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
}
#endif

} // namespace fb::protocol::game::request
