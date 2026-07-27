#ifndef __PROTOCOL_GAME_MOVE_CONFIRM_H__
#define __PROTOCOL_GAME_MOVE_CONFIRM_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class move_confirm : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0B;

public:
#ifndef BOT
    const DIRECTION                  direction;
    const fb::model::point<uint16_t> position;
    const fb::model::point<uint16_t> viewport;
#else
    DIRECTION                  direction;
    fb::model::point<uint16_t> position;
    fb::model::point<uint16_t> viewport;
#endif

public:
#ifndef BOT
    move_confirm(DIRECTION                         direction,
                 const fb::model::point<uint16_t>& position,
                 const fb::model::point<uint16_t>& viewport);
    move_confirm(const fb::game::object&           object,
                 const fb::model::point<uint16_t>& position,
                 const fb::model::point<uint16_t>& viewport);
#else
    move_confirm() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
