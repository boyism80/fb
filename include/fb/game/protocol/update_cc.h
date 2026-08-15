#ifndef __PROTOCOL_GAME_UPDATE_CC_H__
#define __PROTOCOL_GAME_UPDATE_CC_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/life.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class update_cc : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x24;

public:
#ifndef BOT
    const fb::game::life& life;
#else
    CROWD_CONTROL cc = CROWD_CONTROL::NONE;
#endif

public:
#ifndef BOT
    explicit update_cc(const fb::game::life& value);
#else
    update_cc() = default;
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
