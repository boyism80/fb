#include <fb/game/protocol/map/worlds.h>

using table = fb::model::table;
using namespace fb::protocol::game::response;

#ifndef BOT
map_worlds::map_worlds(uint32_t id, uint16_t index) :
    id(id),
    index(index)
{ }
#endif

#ifndef BOT
void map_worlds::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);

    auto  world_attribute_table = table::world_attribute;
    auto& attr                  = world_attribute_table[this->id];
    auto  world_table           = table::world;
    auto& points                = world_table[this->id];

    writer.write<std::string, uint8_t>(attr.key);
    writer.write<uint8_t>(static_cast<uint8_t>(points.size()));
    writer.write<uint8_t>(this->index);

    for (int i = 0; i < points.size(); i++)
    {
        auto& point = points[i];
        writer.write<uint16_t>(point.offset.x);
        writer.write<uint16_t>(point.offset.y);
        writer.write<std::string, uint8_t>(point.name);
        writer.write<uint16_t>(static_cast<uint16_t>(this->id)); // world_value → 0x3F value
        writer.write<uint16_t>(this->id);
        writer.write<uint16_t>(this->index);
        writer.write<uint16_t>(i);
        writer.write<uint16_t>(static_cast<uint16_t>(point.links.size()));

        for (auto x : point.links)
        {
            writer.write<uint16_t>(x);
        }
    }
}
#else
void map_worlds::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->key         = reader.read<std::string, uint8_t>();
    this->world_count = reader.read<uint8_t>();
    this->index       = reader.read<uint8_t>();

    // Fully implement world point structure
    this->points.clear();
    for (int i = 0; i < this->world_count; i++)
    {
        world_point point;
        point.offset_x    = reader.read<uint16_t>();
        point.offset_y    = reader.read<uint16_t>();
        point.name        = reader.read<std::string, uint8_t>();
        point.world_value = reader.read<uint16_t>();
        point.world_id    = reader.read<uint16_t>();
        point.index       = reader.read<uint16_t>();
        point.point_id    = reader.read<uint16_t>();

        uint16_t group_count = reader.read<uint16_t>();
        point.group_points.clear();
        for (int j = 0; j < group_count; j++)
        {
            uint16_t group_point_id = reader.read<uint16_t>();
            point.group_points.push_back(group_point_id);
        }

        this->points.push_back(point);
    }
}
#endif
