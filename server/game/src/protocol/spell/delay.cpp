#include <fb/game/protocol/spell/delay.h>

namespace fb::protocol::game::response {

#ifndef BOT
spell_delay::spell_delay(const fb::game::spell& spell, uint32_t delay) :
    spell(spell),
    delay(delay)
{ }
#endif

#ifndef BOT
void spell_delay::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint8_t>(this->spell.model().name);
    writer.write<uint32_t>(this->delay);
    writer.write<uint8_t>(0);
}
#else
void spell_delay::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->spell_name = reader.read<std::string, uint8_t>();
    this->delay      = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0
}
#endif

} // namespace fb::protocol::game::response