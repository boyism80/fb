#ifndef __PROTOCOL_GAME_OBJECT_DIRECTION_H__
#define __PROTOCOL_GAME_OBJECT_DIRECTION_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class direction : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x11;

public:
#ifndef BOT
    DIRECTION value;
#else
    const DIRECTION value;
#endif

public:
#ifndef BOT
    direction() = default;
#else
    direction(DIRECTION value);
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

class direction : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x11;

public:
#ifndef BOT
    const uint32_t  sequence;
    const DIRECTION value;
#else
    uint32_t  sequence;
    DIRECTION value;
#endif

public:
#ifndef BOT
    direction(const fb::game::object& object);
    direction(uint32_t sequence, DIRECTION value);
#else
    direction() = default;
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