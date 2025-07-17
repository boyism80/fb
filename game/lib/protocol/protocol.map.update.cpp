#include <fb/game/protocol/map/map_update.h>

namespace fb::protocol::game::request {

#ifndef BOT // server only
async::task<void> map_update::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->position.x  = reader.read<uint16_t>();
    this->position.y  = reader.read<uint16_t>();
    this->size.width  = reader.read<uint8_t>();
    this->size.height = reader.read<uint8_t>();
    this->crc         = reader.read<uint16_t>();
}
#else // bot only
async::task<void> map_update::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint16_t>(this->position_x);
    writer.write<uint16_t>(this->position_y);
    writer.write<uint8_t>(this->width);
    writer.write<uint8_t>(this->height);
    writer.write<uint16_t>(this->crc);
}
#endif

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifdef BOT
async::task<void> map_update::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);

    uint8_t effect_flag = reader.read<uint8_t>();
    if (effect_flag == 0x04)
    {
        this->effect = reader.read<uint8_t>();
    }
    else
    {
        this->effect = 0; // NONE
    }

    this->position_x = reader.read<uint16_t>();
    this->position_y = reader.read<uint16_t>();
    this->width      = reader.read<uint8_t>();
    this->height     = reader.read<uint8_t>();

    // Fully implement map tile data
    this->tiles.clear();
    uint32_t tile_count = this->width * this->height;

    for (uint32_t i = 0; i < tile_count; i++)
    {
        tile_data tile;
        tile.id      = reader.read<uint16_t>();
        tile.blocked = reader.read<uint16_t>();
        tile.object  = reader.read<uint16_t>();
        this->tiles.push_back(tile);
    }
}
#endif

} // namespace fb::protocol::game::response
