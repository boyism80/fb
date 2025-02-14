#include <fb/game/protocol/spell/spell_delay.h>

namespace fb::protocol::game::response {

#ifndef BOT
spell_delay::spell_delay(const fb::game::spell& spell, uint32_t delay) :
    spell(spell),
    delay(delay)
{ }
#endif

#ifndef BOT
async::task<void> spell_delay::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<std::string, uint8_t>(this->spell.model.name);
    writer.write<uint32_t>(this->delay);
    writer.write<uint8_t>(0);
}
#else
async::task<void> spell_delay::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response