#include <fb/game/protocol/unknown_26.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> unknown_26::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->flags);
    writer.write<int16_t>(this->pos_x);
    writer.write<int16_t>(this->pos_y);
    writer.write<int16_t>(this->map_rel_x);
    writer.write<int16_t>(this->map_rel_y);
    writer.write<uint8_t>(this->zone_slot & 0x7F);
}
#else
async::task<void> unknown_26::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->flags     = reader.read<uint8_t>();
    this->pos_x     = reader.read<int16_t>();
    this->pos_y     = reader.read<int16_t>();
    this->map_rel_x = reader.read<int16_t>();
    this->map_rel_y = reader.read<int16_t>();
    this->zone_slot = reader.read<uint8_t>() & 0x7F;
}
#endif

} // namespace fb::protocol::game::response
