#include <fb/game/protocol/object/sound.h>

namespace fb::protocol::game::response {

#ifndef BOT
sound::sound(const fb::game::object& me, SOUND value, uint8_t volume) :
    me(me),
    value(value),
    volume(volume)
{ }
#endif

#ifndef BOT
void sound::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(AUDIO::SOUND));
    writer.write<uint8_t>(TLV_SIZE_SOUND);
    writer.write<uint16_t>(static_cast<uint16_t>(this->value));
    writer.write<uint8_t>(this->volume);
    // Target TLV type0 + len4 → oid; then effect/extra TLV constants consumed by client.
    writer.write<uint16_t>(0x0004);
    writer.write<uint32_t>(this->me.oid());
    writer.write<uint16_t>(0x0100);
    writer.write<uint16_t>(0x0202);
    writer.write<uint16_t>(0x0004);
    writer.write<uint16_t>(0xCCCC);
}
#else
void sound::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // AUDIO::SOUND
    reader.read<uint8_t>(); // TLV_SIZE_SOUND
    this->value  = static_cast<SOUND>(reader.read<uint16_t>());
    this->volume = reader.read<uint8_t>();
    reader.read<uint16_t>(); // target TLV header
    this->oid = reader.read<uint32_t>();
    reader.read<uint16_t>();
    reader.read<uint16_t>();
    reader.read<uint16_t>();
    reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
