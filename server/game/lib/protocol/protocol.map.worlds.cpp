#include <fb/game/protocol/map/map_worlds.h>

namespace fb::protocol::game::response {

#ifndef BOT
map_worlds::map_worlds(const fb::model::model& model, uint32_t id, uint16_t index) :
    model(model),
    id(id),
    index(index)
{ }
#endif

#ifndef BOT
async::task<void> map_worlds::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);

    auto& attr   = this->model.world_attribute[this->id];
    auto& points = this->model.world[this->id];
    auto  g      = std::unordered_map<uint32_t, std::vector<uint16_t>>();
    for (auto& [id, point] : points)
    {
        if (g.contains(point.group))
            g[point.group].push_back(id);
        else
            g.insert({point.group, std::vector<uint16_t>{id}});
    }

    writer.write<std::string, uint8_t>(attr.key);
    writer.write<uint8_t>(this->model.world[this->id].size());
    writer.write<uint8_t>(this->index);

    for (int i = 0; i < points.size(); i++)
    {
        auto& point = this->model.world[this->id][i];
        writer.write<uint16_t>(point.offset.x);
        writer.write<uint16_t>(point.offset.y);
        writer.write<std::string, uint8_t>(point.name);
        writer.write<uint16_t>(0x0000);
        writer.write<uint16_t>(this->id);
        writer.write<uint16_t>(this->index);
        writer.write<uint16_t>(i);
        writer.write<uint16_t>(g[point.group].size());

        for (auto x : g[point.group])
        {
            writer.write<uint16_t>(x);
        }
    }
}
#else
async::task<void> map_worlds::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->key         = reader.read<std::string, uint8_t>();
    this->world_count = reader.read<uint8_t>();
    this->index       = reader.read<uint8_t>();

    // Fully implement world point structure
    this->points.clear();
    for (int i = 0; i < this->world_count; i++)
    {
        world_point point;
        point.offset_x = reader.read<uint16_t>();
        point.offset_y = reader.read<uint16_t>();
        point.name     = reader.read<std::string, uint8_t>();
        point.unknown1 = reader.read<uint16_t>(); // 0x0000
        point.world_id = reader.read<uint16_t>();
        point.index    = reader.read<uint16_t>();
        point.point_id = reader.read<uint16_t>();

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

} // namespace fb::protocol::game::response
