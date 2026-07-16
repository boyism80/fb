#include <fb/game/protocol/loot.h>

namespace fb::protocol::game::request {

#ifndef BOT
void loot::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->boost = bool(reader.read<uint8_t>());
}
#else
loot::loot(bool boost) :
    boost(boost)
{ }

void loot::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->boost);
}
#endif

} // namespace fb::protocol::game::request
