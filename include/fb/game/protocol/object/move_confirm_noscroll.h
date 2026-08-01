#ifndef __PROTOCOL_GAME_MOVE_CONFIRM_NOSCROLL_H__
#define __PROTOCOL_GAME_MOVE_CONFIRM_NOSCROLL_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x26 — move confirm variant used while CharStats / overlay UI is open.
 * Wire layout matches move_confirm (0x0B); client often skips OnPacket_Position (no scroll).
 */
class move_confirm_noscroll : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x26;

public:
#ifndef BOT
    const DIRECTION                  direction;
    const fb::model::point<uint16_t> position;
    const fb::model::point<uint16_t> viewport;
    const uint8_t                    walk_queue_slot;
#else
    DIRECTION                  direction;
    fb::model::point<uint16_t> position;
    fb::model::point<uint16_t> viewport;
    uint8_t                    walk_queue_slot = 0;
#endif

public:
#ifndef BOT
    move_confirm_noscroll(DIRECTION                         direction,
                          const fb::model::point<uint16_t>& position,
                          const fb::model::point<uint16_t>& viewport,
                          uint8_t                           walk_queue_slot);
    move_confirm_noscroll(const fb::game::object&           object,
                          const fb::model::point<uint16_t>& position,
                          const fb::model::point<uint16_t>& viewport,
                          uint8_t                           walk_queue_slot);
#else
    move_confirm_noscroll() = default;
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
