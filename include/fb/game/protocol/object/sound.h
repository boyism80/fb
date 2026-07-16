#ifndef __PROTOCOL_GAME_SOUND_H__
#define __PROTOCOL_GAME_SOUND_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class sound : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x19;

public:
#ifndef BOT
    const fb::game::object& me;
    const SOUND             value;
#else
    uint32_t oid;
    SOUND    value;
#endif

public:
#ifndef BOT
    sound(const fb::game::object& me, SOUND value);
#else
    sound() = default;
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