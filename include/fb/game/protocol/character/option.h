#ifndef __PROTOCOL_GAME_OPTION_H__
#define __PROTOCOL_GAME_OPTION_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class option : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x23;

public:
#ifdef BOT
    bool weather_effect = false;
    bool magic_effect   = false;
    bool roar_worlds    = false;
    bool fast_move      = false;
    bool effect_sound   = false;
#else
    const fb::game::character& ch;
#endif

public:
#ifdef BOT
    option() = default;
#else
    option(const fb::game::character& ch);
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