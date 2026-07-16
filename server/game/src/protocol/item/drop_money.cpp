#include <fb/game/protocol/item/drop_money.h>

namespace fb::protocol::game::request {

#ifdef BOT
void item_drop_money::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->chunk);
}
#else
void item_drop_money::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->chunk = reader.read<uint32_t>();
}
#endif
} // namespace fb::protocol::game::request
