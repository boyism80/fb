#include <fb/game/protocol/character/id.h>

namespace fb::protocol::game::response {

#ifndef BOT
id::id(const fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
void id::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->ch.oid());
    // Client uses only the low byte of this BE u32 as direction.
    writer.write<uint32_t>(static_cast<uint32_t>(this->ch.direction()));
    writer.write<uint8_t>(static_cast<uint8_t>(this->ch.cls()));
}
#else
void id::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid       = reader.read<uint32_t>();
    this->direction = reader.read<uint32_t>();
    this->cls       = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
