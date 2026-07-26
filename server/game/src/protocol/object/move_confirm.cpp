#include <fb/game/protocol/object/move_confirm.h>

using namespace fb::model;

namespace fb::protocol::game::response {

#ifndef BOT
move_confirm::move_confirm(DIRECTION direction, const point<uint16_t>& position) :
    direction(direction),
    position(position)
{ }

move_confirm::move_confirm(const fb::game::object& object, const point<uint16_t>& position) :
    move_confirm(object.direction(), position)
{ }

void move_confirm::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    // Map-relative pair; match path only needs abs x/y/dir, mirror abs for safety.
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint8_t>(0x01);
}
#else
void move_confirm::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->direction  = static_cast<DIRECTION>(reader.read<uint8_t>());
    this->position.x = reader.read<uint16_t>();
    this->position.y = reader.read<uint16_t>();
    reader.read<uint16_t>();
    reader.read<uint16_t>();
    reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
