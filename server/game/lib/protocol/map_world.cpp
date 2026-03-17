#include <fb/game/protocol/map/map_world.h>

namespace fb::protocol::game::request {

#ifndef BOT // server only
async::task<void> map_world::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->value  = reader.read<uint16_t>();
    this->before = reader.read<uint16_t>();
    this->after  = reader.read<uint16_t>();
}
#else // bot only
async::task<void> map_world::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->value);
    writer.write<uint16_t>(this->before);
    writer.write<uint16_t>(this->after);
}
#endif

} // namespace fb::protocol::game::request
