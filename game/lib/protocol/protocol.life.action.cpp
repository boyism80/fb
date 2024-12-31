#include <fb/game/protocol/life/action.h>

namespace fb::protocol::game::response {

#ifndef BOT
action::action(const fb::game::life& me, ACTION value, DURATION duration, uint8_t sound) :
    me(me),
    value(value),
    duration(duration),
    sound(sound)
{ }
#endif

#ifndef BOT
async::task<void> action::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->me.sequence());
    writer.write<uint8_t>(static_cast<uint8_t>(this->value));      // type
    writer.write<uint16_t>(static_cast<uint16_t>(this->duration)); // duration
    writer.write<uint8_t>(this->sound);                            // sound
}
#else
async::task<void> action::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->sequence = reader.read<uint32_t>();
    this->value    = (ACTION)reader.read<uint8_t>();
    this->duration = (DURATION)reader.read<uint16_t>();
    this->sound    = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
