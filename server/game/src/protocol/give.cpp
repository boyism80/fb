#include <fb/game/protocol/give.h>

namespace fb::protocol::game::request {

#ifdef BOT
void give_item::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->slot + 1);
    writer.write<bool>(this->all);
}
#else
void give_item::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->slot = reader.read<uint8_t>() - 1;
    this->all  = reader.read<bool>();
}
#endif

#ifdef BOT
void give_money::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->money);
}
#else
void give_money::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->money = reader.read<uint32_t>();
}
#endif
} // namespace fb::protocol::game::request
