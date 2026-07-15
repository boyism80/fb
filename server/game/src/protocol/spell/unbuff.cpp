#include <fb/game/protocol/spell/unbuff.h>

namespace fb::protocol::game::response {

#ifndef BOT
spell_unbuff::spell_unbuff(const fb::game::buff& buff) :
    buff(buff)
{ }
#endif

#ifndef BOT
void spell_unbuff::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string>(this->buff.model.name);
    writer.write<uint32_t>(0x00);
}
#else
void spell_unbuff::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->buff_name = reader.read<std::string, uint8_t>();
    reader.read<uint32_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
