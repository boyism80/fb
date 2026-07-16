#include <fb/game/protocol/object/move.h>

using namespace fb::model;

namespace fb::protocol::game::request {

#ifndef BOT
void move::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->direction  = DIRECTION(reader.read<uint8_t>());
    this->oid        = reader.read<uint8_t>();
    this->position.x = reader.read<uint16_t>();
    this->position.y = reader.read<uint16_t>();
}
#else
move::move(DIRECTION direction, uint32_t oid, fb::model::point<uint16_t> position) :
    direction(direction),
    oid(oid),
    position(position)
{ }

void move::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
    writer.write<uint8_t>(this->oid);
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
}
#endif

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
move::move(const fb::game::object& object, const point<uint16_t>& position) :
    move(object.oid(), object.direction(), position)
{ }
move::move(const uint32_t id, DIRECTION direction, const point<uint16_t>& position) :
    id(id),
    direction(direction),
    position(position)
{ }
#endif

#ifndef BOT
void move::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->id);
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
    writer.write<uint8_t>(0x00);
}
#else
void move::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->id         = reader.read<uint32_t>();
    this->position.x = reader.read<uint16_t>();
    this->position.y = reader.read<uint16_t>();
    this->direction  = (DIRECTION)reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response