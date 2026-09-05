#ifndef __PROTOCOL_GAME_CHARACTER_UPDATE_EXTERNAL_H__
#define __PROTOCOL_GAME_CHARACTER_UPDATE_EXTERNAL_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#include <fb/game/appearance.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

#ifndef BOT
HEAD_MARKER compute_head_marker(const fb::game::character& ch, const fb::game::object& to);
#endif

template <CLIENT_VERSION V = CLIENT_VERSION::v550>
class update_external : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x1D;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    uint32_t                          oid;
    HEAD_MARKER                       head_marker;
    std::string                       name;
    fb::game::character_appearance<V> appearance;
    CLIENT_UI_MODE                    ui_mode = CLIENT_UI_MODE::OLD;
#else
    uint32_t    oid;
    uint8_t     disguised;
    GENDER      gender;
    STATE       state;
    uint16_t    look;
    uint8_t     color;
    uint8_t     armor_dress;
    uint8_t     armor_color;
    uint16_t    weapon_dress;
    uint8_t     weapon_color;
    uint8_t     shield_dress;
    uint8_t     shield_color;
    uint8_t     head_marker;
    std::string name;
#endif

public:
#ifndef BOT
    update_external(const fb::game::character& ch, const fb::game::object& to);
    update_external(uint32_t                                 oid,
                    HEAD_MARKER                              head_marker,
                    std::string                              name,
                    const fb::game::character_appearance<V>& appearance,
                    CLIENT_UI_MODE                           ui_mode);
    update_external(const update_external&) = delete;
#else
    update_external() = default;
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
void update_external<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#else
template <>
void update_external<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

using update_external_v550 = update_external<CLIENT_VERSION::v550>;
using update_external_v565 = update_external<CLIENT_VERSION::v565>;
using update_external_v651 = update_external<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
