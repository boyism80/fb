#ifndef __PROTOCOL_GAME_OBJECT_SHOW_H__
#define __PROTOCOL_GAME_OBJECT_SHOW_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class update : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x07;

public:
#ifndef BOT
    const std::vector<fb::game::object*> objects;
#else
    struct object_data
    {
        uint16_t x;
        uint16_t y;
        uint32_t oid;
        uint16_t look;
        uint8_t  color;
        uint8_t  direction;
    };

    uint16_t                 object_count;
    std::vector<object_data> objects_data;
#endif

public:
#ifndef BOT
    update(const fb::game::object& object);
    update(const std::vector<fb::game::object*>& objects);
#else
    update() = default;
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