#include <fb/game/protocol/holyday_screen.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> holyday_screen::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint16_t>(this->screen);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(this->hair);
    writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
    writer.write<uint8_t>(this->position.x);
    writer.write<uint8_t>(this->position.y);
}
#else
async::task<void> holyday_screen::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->screen = reader.read<uint8_t>();
    (void)reader.read<uint8_t>(); // pad
    this->hair       = reader.read<uint8_t>();
    this->direction  = static_cast<DIRECTION>(reader.read<uint8_t>());
    this->position.x = reader.read<uint8_t>();
    this->position.y = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
