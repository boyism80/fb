#include <fb/game/protocol/character/option.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
option<V>::option(const fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
template <CLIENT_VERSION V>
void option<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->ch.option(OPTION::WEATHER_EFFECT));
    writer.write<uint8_t>(this->ch.option(OPTION::MAGIC_EFFECT));
    writer.write<uint8_t>(this->ch.option(OPTION::NEWS));
    writer.write<uint8_t>(this->ch.option(OPTION::FAST_MOVE));
    writer.write<uint8_t>(this->ch.option(OPTION::EFFECT_SOUND));
}

template <>
void option<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->ch.option(OPTION::WEATHER_EFFECT));
    writer.write<uint8_t>(this->ch.option(OPTION::MAGIC_EFFECT));
    writer.write<uint8_t>(this->ch.option(OPTION::NEWS));
    writer.write<uint8_t>(this->ch.option(OPTION::FAST_MOVE));
    writer.write<uint8_t>(this->ch.option(OPTION::EFFECT_SOUND));
    writer.write<uint8_t>(this->ch.option(OPTION::VISIBLE_HELMET));
}
#else
template <CLIENT_VERSION V>
void option<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->weather_effect = reader.read<uint8_t>();
    this->magic_effect   = reader.read<uint8_t>();
    this->news           = reader.read<uint8_t>();
    this->fast_move      = reader.read<uint8_t>();
    this->effect_sound   = reader.read<uint8_t>();
}

template <>
void option<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->weather_effect = reader.read<uint8_t>();
    this->magic_effect   = reader.read<uint8_t>();
    this->news           = reader.read<uint8_t>();
    this->fast_move      = reader.read<uint8_t>();
    this->effect_sound   = reader.read<uint8_t>();
    this->visible_helmet = reader.read<uint8_t>();
}
#endif

template class option<CLIENT_VERSION::v550>;
template class option<CLIENT_VERSION::v565>;
template class option<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
