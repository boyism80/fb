#ifndef __PROTOCOL_GAME_SPELL_UPDATE_H__
#define __PROTOCOL_GAME_SPELL_UPDATE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/life.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class spell_update : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x17;

public:
#ifdef BOT
    uint8_t     index;
    uint8_t     type;
    std::string name;
    std::string message;
#else
    const fb::game::life& me;
    const uint8_t         index;
#endif

public:
#ifndef BOT
    spell_update(const fb::game::life& me, uint8_t index);
#else
    spell_update() = default;
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