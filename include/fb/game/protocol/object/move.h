#ifndef __PROTOCOL_GAME_MOVE_H__
#define __PROTOCOL_GAME_MOVE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class move : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x32;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    DIRECTION                  direction;
    uint8_t                    walk_queue_slot = 0; // client circular walk-queue index (0..63)
    fb::model::point<uint16_t> position;

public:
#ifndef BOT
    move() = default;
#else
    move(DIRECTION direction, uint32_t walk_queue_slot, fb::model::point<uint16_t> position);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class move : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0C;

public:
#ifndef BOT
    const uint32_t                   id;
    const fb::model::point<uint16_t> position;
    const DIRECTION                  direction;
#else
    uint32_t                   id;
    fb::model::point<uint16_t> position;
    DIRECTION                  direction;
#endif

public:
#ifndef BOT
    move(const fb::game::object& object, const fb::model::point<uint16_t>& position);
    move(const uint32_t id, DIRECTION direction, const fb::model::point<uint16_t>& position);
#else
    move() = default;
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