#ifndef __PROTOCOL_GAME_EFFECT_H__
#define __PROTOCOL_GAME_EFFECT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class effect : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x29;

public:
#ifndef BOT
    const fb::game::object& me;
    const uint8_t           value;
#else
    uint32_t oid;
    uint8_t  value;
#endif

public:
#ifndef BOT
    effect(const fb::game::object& me, uint8_t value);
#else
    effect() = default;
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