#ifndef __PROTOCOL_GAME_HOLYDAY_SCREEN_H__
#define __PROTOCOL_GAME_HOLYDAY_SCREEN_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;
using point8_t = fb::model::point<uint8_t>;

class holyday_screen : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x5A;

public:
#ifndef BOT
    const uint8_t   screen;
    const uint8_t   hair;
    const DIRECTION direction;
    const point8_t  position;
#else
    uint8_t   screen;
    uint8_t   hair;
    DIRECTION direction;
    point8_t  position;
#endif

public:
#ifndef BOT
    holyday_screen(uint8_t screen, uint8_t hair, DIRECTION direction, const point8_t& position) :
        screen(screen),
        hair(hair),
        direction(direction),
        position(position)
    { }
#else
    holyday_screen() = default;
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
