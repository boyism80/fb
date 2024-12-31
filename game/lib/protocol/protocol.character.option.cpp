#include <fb/game/protocol/character/option.h>

namespace fb::protocol::game::response {

#ifndef BOT
option::option(const fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
async::task<void> option::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->ch.option(SETTING::WEATHER_EFFECT)); // weather
    writer.write<uint8_t>(this->ch.option(SETTING::MAGIC_EFFECT));   // magic effect
    writer.write<uint8_t>(this->ch.option(SETTING::ROAR_WORLDS));    // listen news
    writer.write<uint8_t>(this->ch.option(SETTING::FAST_MOVE));      // fast move
    writer.write<uint8_t>(this->ch.option(SETTING::EFFECT_SOUND));   // effect sound
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> option::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->weather_effect = reader.read<uint8_t>();
    this->magic_effect   = reader.read<uint8_t>();
    this->roar_worlds    = reader.read<uint8_t>();
    this->fast_move      = reader.read<uint8_t>();
    this->effect_sound   = reader.read<uint8_t>();
    reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
