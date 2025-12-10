#ifndef __PROTOCOL_GAME_SPELL_DELAY_H__
#define __PROTOCOL_GAME_SPELL_DELAY_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/life.h>
#include <fb/game/spell.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class spell_delay : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x3A;

public:
#ifndef BOT
    const fb::game::spell& spell;
    const uint32_t         delay;
#else
    std::string spell_name;
    uint32_t    delay;
#endif

public:
#ifndef BOT
    spell_delay(const fb::game::spell& spell, uint32_t delay);
#else
    spell_delay() = default;
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