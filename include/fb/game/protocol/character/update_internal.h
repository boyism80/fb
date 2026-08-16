#ifndef __PROTOCOL_GAME_STATE_H__
#define __PROTOCOL_GAME_STATE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <cstdint>
#ifndef BOT
#include <fb/game/character.h>
#else
#include <macro.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class update_internal : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x08;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT
    UPDATE_STATE_LEVEL level;
    bool               ch_follow_camera   = false;
    bool               ch_lock_walk_speed = false;
    bool               ch_gm              = false;
    uint16_t           ch_nation          = 0;
    uint8_t            ch_divine_beast    = 0;
    uint8_t            unknown_based_5    = 0;
    uint8_t            ch_level           = 0;
    uint32_t           ch_base_hp         = 0;
    uint32_t           ch_base_mp         = 0;
    uint8_t            ch_strength        = 0;
    uint8_t            ch_intelligence    = 0;
    uint8_t            ch_dexterity       = 0;
    int16_t            reputation         = 0;
    uint16_t           evaluation         = 0;
    uint32_t           ch_hp              = 0;
    uint32_t           ch_mp              = 0;
    uint32_t           ch_exp             = 0;
    uint32_t           ch_money           = 0;
    uint32_t           ch_crowd_control   = 0;
    uint8_t            ch_mail            = 0;
    uint8_t            ch_fast_move       = 0;
    uint8_t            ch_speed           = 0;
    uint8_t            unknown_exp_pad    = 0;
    uint32_t           option_bits        = 0;
#else
    const fb::game::character& ch;
    const UPDATE_STATE_LEVEL   level;
    uint8_t                    unknown_based_5 = 0;
    int16_t                    reputation      = 0;
    uint16_t                   evaluation      = 0;
    uint8_t                    unknown_exp_pad = 0;
#endif

public:
#ifdef BOT
    update_internal() = default;
#else
    update_internal(const fb::game::character& ch, UPDATE_STATE_LEVEL level);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

#ifndef BOT
template <>
void update_internal<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#else
template <>
void update_internal<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

using update_internal_v550 = update_internal<CLIENT_VERSION::v550>;
using update_internal_v565 = update_internal<CLIENT_VERSION::v565>;
using update_internal_v651 = update_internal<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
