#ifndef __PROTOCOL_GAME_LIFE_ACTION_H__
#define __PROTOCOL_GAME_LIFE_ACTION_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/life.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class action : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x1A;

public:
#ifndef BOT
    const fb::game::life& me;
    const ACTION          value;
    const DURATION        duration;
    const uint8_t         sound;
#else
    uint32_t sequence;
    ACTION   value;
    DURATION duration;
    uint8_t  sound;
#endif

public:
#ifndef BOT
    action(const fb::game::life& me, ACTION value, DURATION duration, uint8_t sound = 0x00);
#else
    action() = default;
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