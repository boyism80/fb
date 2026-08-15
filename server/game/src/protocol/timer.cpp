#include <fb/game/protocol/timer.h>

namespace fb::protocol::game::response {

#ifndef BOT
timer::timer(uint32_t time, TIMER_TYPE type) :
    time(time),
    type(type)
{ }
#endif

#ifndef BOT
void timer::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<uint32_t>(this->time);
    writer.write<uint8_t>(0x00);
}
#else
void timer::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = static_cast<TIMER_TYPE>(reader.read<uint8_t>());
    this->time = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
