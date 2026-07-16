#include <fb/game/protocol/time.h>

namespace fb::protocol::game::response {

#ifndef BOT
void time::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->hours % 24); // hours
    writer.write<uint8_t>(0x00);             // Unknown
    writer.write<uint8_t>(0x00);             // Unknown
}
#else
void time::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->hours = reader.read<uint8_t>();
    reader.read<uint8_t>();
    reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
