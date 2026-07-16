#include <fb/game/protocol/weather.h>

namespace fb::protocol::game::response {

#ifndef BOT
void weather::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->value));
    writer.write<uint8_t>(0x00);
}
#else
void weather::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->value = static_cast<WEATHER_TYPE>(reader.read<uint8_t>());
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
