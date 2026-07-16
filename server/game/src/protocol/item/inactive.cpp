#include <fb/game/protocol/item/inactive.h>

namespace fb::protocol::game::request {

#ifdef BOT
void item_inactive::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
}
#else
void item_inactive::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->parts = static_cast<EQUIPMENT_PARTS>(reader.read<uint8_t>());
}
#endif
} // namespace fb::protocol::game::request
