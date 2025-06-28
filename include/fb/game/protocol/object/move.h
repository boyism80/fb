#ifndef __PROTOCOL_GAME_MOVE_H__
#define __PROTOCOL_GAME_MOVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class move : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x32;

public:
    DIRECTION                  direction;
    uint8_t                    sequence;
    fb::model::point<uint16_t> position;

public:
#ifndef BOT
    move() = default;
#else
    move(DIRECTION direction, uint32_t sequence, fb::model::point<uint16_t> position);
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

class move : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x0C;

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
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif