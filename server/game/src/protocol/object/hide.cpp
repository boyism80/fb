#include <fb/game/protocol/object/hide.h>

namespace fb::protocol::game::response {

#ifndef BOT
hide::hide(const fb::game::object& object) :
    hide(object.oid())
{ }
hide::hide(uint32_t oid) :
    oid(oid)
{ }
#endif

#ifndef BOT
void hide::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(0x00);
}
#else
void hide::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
