#include <fb/game/protocol/object/effect.h>

namespace fb::protocol::game::response {

#ifndef BOT
effect::effect(const fb::game::object& me, uint8_t value) :
    me(me),
    value(value)
{ }
#endif

#ifndef BOT
void effect::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->me.oid());
    writer.write<uint8_t>(this->value);
    writer.write<uint8_t>(0x00);
}
#else
void effect::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid   = reader.read<uint32_t>();
    this->value = reader.read<uint8_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
