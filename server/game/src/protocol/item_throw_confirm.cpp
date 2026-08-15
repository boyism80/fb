#include <fb/game/protocol/item_throw_confirm.h>

namespace fb::protocol::game::response {

#ifndef BOT
item_throw_confirm::item_throw_confirm(uint8_t inventory_slot) :
    inventory_slot(inventory_slot)
{ }
#endif

#ifndef BOT
void item_throw_confirm::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->inventory_slot);
}
#else
void item_throw_confirm::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->inventory_slot = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
