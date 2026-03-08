#include <fb/game/protocol/item_throw_confirm.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> item_throw_confirm::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->inventory_slot);
}
#else
async::task<void> item_throw_confirm::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->inventory_slot = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
