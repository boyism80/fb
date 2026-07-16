#ifndef __PROTOCOL_GAME_OBJECT_MOVE_BLOCKED_H__
#define __PROTOCOL_GAME_OBJECT_MOVE_BLOCKED_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

/**
 * Client-to-server packet 0x69 (move blocked).
 * Sent when the character tries to move but is blocked by an obstacle.
 * Payload: current position (x, y), attempted direction. Server may ignore (no-op).
 */
class move_blocked : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x69;

public:
    fb::model::point<uint16_t> position;
    DIRECTION                  direction;

public:
    move_blocked() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
