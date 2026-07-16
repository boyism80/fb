#include <fb/game/protocol/unknown_12.h>

namespace fb::protocol::game::response {

#ifndef BOT
void unknown_12::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->party_slot);
    writer.write<uint8_t>(this->level_encoded);
}
#else
void unknown_12::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid           = reader.read<uint32_t>();
    this->party_slot    = reader.read<uint8_t>();
    this->level_encoded = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
