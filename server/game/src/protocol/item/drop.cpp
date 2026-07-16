#include <fb/game/protocol/item/drop.h>

namespace fb::protocol::game::request {

#ifdef BOT
void item_drop::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index);
    writer.write<bool>(this->all);
}
#else
void item_drop::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
    this->all   = bool(reader.read<uint8_t>());
}
#endif
} // namespace fb::protocol::game::request
