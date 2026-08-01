#include <fb/game/protocol/time.h>

namespace fb::protocol::game::response {

#ifndef BOT
void time::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->hours % 24);
    writer.write<uint8_t>(this->minutes % 60); // used for midnight lighting interpolation
}
#else
void time::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->hours   = reader.read<uint8_t>();
    this->minutes = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
