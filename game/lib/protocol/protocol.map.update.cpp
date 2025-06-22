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

#ifndef BOT // server only
map_update::map_update(const fb::game::map&        map,
                       const fb::model::point16_t& position,
                       const fb::model::size8_t&   size,
                       uint16_t                    crc) :
    map(map),
    position(position),
    size(size),
    crc(crc)
{ }
#endif

#ifndef BOT // server only
async::task<void> map_update::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);

    if (this->map.model.effect == MAP_EFFECT_TYPE::NONE)
    {
        writer.write<uint8_t>(0x00);
    }
    else
    {
        writer.write<uint8_t>(0x04);
        writer.write<uint8_t>(static_cast<uint8_t>(this->map.model.effect));
    }

    writer.write<uint16_t>(this->position.x);
    writer.write<uint16_t>(this->position.y);
    writer.write<uint8_t>(this->size.width);
    writer.write<uint8_t>(this->size.height);

    uint16_t now_crc  = 0;
    uint32_t map_size = this->size.width * this->size.height * sizeof(uint16_t) * 3; // tile id, block, object
    for (int row = this->position.y; row < this->position.y + this->size.height; row++)
    {
        for (int col = this->position.x; col < this->position.x + this->size.width; col++)
        {
            auto tile = this->map(col, row);
            if (tile == nullptr)
                continue;

            writer.write<uint16_t>(tile->id);
            writer.write<uint16_t>(tile->blocked);
            writer.write<uint16_t>(tile->object);

            now_crc = (now_crc << 8) ^ CRC16_TAB[now_crc >> 8] ^ tile->id;
            now_crc = (now_crc << 8) ^ CRC16_TAB[now_crc >> 8] ^ uint16_t(tile->blocked);
            now_crc = (now_crc << 8) ^ CRC16_TAB[now_crc >> 8] ^ tile->object;
        }
    }

    if (crc == now_crc)
        co_return;
}
#else // bot only
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
