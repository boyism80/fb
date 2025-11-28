#ifndef __PROTOCOL_GAME_WORLDS_H__
#define __PROTOCOL_GAME_WORLDS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class map_worlds : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x2E;

#ifdef BOT

public:
    struct world_point
    {
        uint16_t              offset_x;
        uint16_t              offset_y;
        std::string           name;
        uint16_t              unknown1;
        uint16_t              world_id;
        uint16_t              index;
        uint16_t              point_id;
        std::vector<uint16_t> group_points;
    };
#endif

public:
#ifndef BOT
    const uint32_t id;
    const uint16_t index;
#else
    std::string              key;
    uint8_t                  world_count;
    uint16_t                 index;
    std::vector<world_point> points;
#endif

public:
#ifndef BOT
    map_worlds(uint32_t id, uint16_t index);
#else
    map_worlds() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif