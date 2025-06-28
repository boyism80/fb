#ifndef __PROTOCOL_GAME_SPELL_REMOVE_H__
#define __PROTOCOL_GAME_SPELL_REMOVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/life.h>
#include <fb/game/spell.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class spell_remove : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x18;

public:
#ifndef BOT
    const fb::game::life& me;
    const uint8_t         index;
#else
    uint8_t index;
#endif

public:
#ifndef BOT
    spell_remove(const fb::game::life& me, uint8_t index);
#else
    spell_remove() = default;
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