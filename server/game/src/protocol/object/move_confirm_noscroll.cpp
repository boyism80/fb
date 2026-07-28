#include <fb/game/protocol/object/move_confirm_noscroll.h>

using namespace fb::model;

namespace fb::protocol::game::response {

#ifndef BOT
move_confirm_noscroll::move_confirm_noscroll(DIRECTION              direction,
                                             const point<uint16_t>& position,
                                             const point<uint16_t>& viewport,
                                             uint8_t                walk_queue_slot) :
    direction(direction),
    position(position),
    viewport(viewport),
    walk_queue_slot(walk_queue_slot)
{ }

move_confirm_noscroll::move_confirm_noscroll(const fb::game::object& object,
                                             const point<uint16_t>&  position,
                                             const point<uint16_t>&  viewport,
                                             uint8_t                 walk_queue_slot) :
    move_confirm_noscroll(object.direction(), position, viewport, walk_queue_slot)
{ }

void move_confirm_noscroll::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint16_t>(this->viewport.x);
    writer.write<uint16_t>(this->viewport.y);
    writer.write<uint8_t>(this->walk_queue_slot & 0x7F);
}
#else
void move_confirm_noscroll::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->direction       = static_cast<DIRECTION>(reader.read<uint8_t>());
    this->position.x      = reader.read<uint16_t>();
    this->position.y      = reader.read<uint16_t>();
    this->viewport.x      = reader.read<uint16_t>();
    this->viewport.y      = reader.read<uint16_t>();
    this->walk_queue_slot = reader.read<uint8_t>() & 0x7F;
}
#endif

} // namespace fb::protocol::game::response
