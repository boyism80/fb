#include <fb/game/protocol/item/item_remove.h>

namespace fb::protocol::game::response {

#ifndef BOT
item_remove::item_remove(ITEM_DELETE_TYPE type, uint32_t index, uint16_t count) :
    type(type),
    index(index),
    count(count)
{ }
#endif

#ifndef BOT
async::task<void> item_remove::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<uint16_t>(this->count);
}
#else
async::task<void> item_remove::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1; // Client sends 1-based index
    this->type  = static_cast<ITEM_DELETE_TYPE>(reader.read<uint8_t>());
    this->count = reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
