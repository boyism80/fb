#ifndef __PROTOCOL_GAME_EFFECT_H__
#define __PROTOCOL_GAME_EFFECT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class effect : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x29;

public:
#ifndef BOT
    const fb::game::object& me;
    const uint8_t           value;
#else

#endif

public:
#ifndef BOT
    effect(const fb::game::object& me, uint8_t value);
#else
    effect() = default;
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