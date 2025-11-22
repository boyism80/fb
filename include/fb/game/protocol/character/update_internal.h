#ifndef __PROTOCOL_GAME_STATE_H__
#define __PROTOCOL_GAME_STATE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#else
#include <macro.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class update_internal : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x08;

public:
#ifdef BOT
    UPDATE_STATE_LEVEL level;
    uint8_t            ch_nation        = 0;
    uint8_t            ch_creature      = 0;
    uint8_t            ch_level         = 0;
    uint32_t           ch_base_hp       = 0;
    uint32_t           ch_base_mp       = 0;
    uint8_t            ch_strength      = 0;
    uint8_t            ch_intelligence  = 0;
    uint8_t            ch_dexterity     = 0;
    uint32_t           ch_hp            = 0;
    uint32_t           ch_mp            = 0;
    uint32_t           ch_exp           = 0;
    uint32_t           ch_money         = 0;
    uint32_t           ch_crowd_control = 0;
    uint8_t            ch_mail          = 0;
    uint8_t            ch_fast_move     = 0;
#else
    const fb::game::character& ch;
    const UPDATE_STATE_LEVEL   level;
#endif

public:
#ifdef BOT
    update_internal() = default;
#else
    update_internal(const fb::game::character& ch, UPDATE_STATE_LEVEL level);
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