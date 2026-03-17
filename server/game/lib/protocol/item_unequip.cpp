#include <fb/game/protocol/item/item_unequip.h>

namespace fb::protocol::game::response {

#ifndef BOT
item_unequip::item_unequip(EQUIPMENT_PARTS parts) :
    parts(parts)
{ }
#endif

#ifndef BOT
async::task<void> item_unequip::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> item_unequip::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->parts = static_cast<EQUIPMENT_PARTS>(reader.read<uint8_t>());
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
