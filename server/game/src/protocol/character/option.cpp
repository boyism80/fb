#include <fb/game/protocol/character/option.h>

namespace fb::protocol::game::response {

#ifndef BOT
option::option(const fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
void option::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->ch.option(OPTION::WEATHER_EFFECT)); // weather
    writer.write<uint8_t>(this->ch.option(OPTION::MAGIC_EFFECT));   // magic effect
    writer.write<uint8_t>(this->ch.option(OPTION::NEWS));           // news
    writer.write<uint8_t>(this->ch.option(OPTION::FAST_MOVE));      // fast move
    writer.write<uint8_t>(this->ch.option(OPTION::EFFECT_SOUND));   // effect sound
}
#else
void option::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->weather_effect = reader.read<uint8_t>();
    this->magic_effect   = reader.read<uint8_t>();
    this->news           = reader.read<uint8_t>();
    this->fast_move      = reader.read<uint8_t>();
    this->effect_sound   = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
