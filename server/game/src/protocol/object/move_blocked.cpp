#include <fb/game/protocol/object/move_blocked.h>

namespace fb::protocol::game::request {

#ifndef BOT
void move_blocked::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->position.x = reader.read<uint16_t>();
    this->position.y = reader.read<uint16_t>();
    this->direction  = DIRECTION(reader.read<uint8_t>());
}
#else
void move_blocked::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
}
#endif
} // namespace fb::protocol::game::request
