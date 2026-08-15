#ifndef __PROTOCOL_GAME_CHARACTER_SHOW_H__
#define __PROTOCOL_GAME_CHARACTER_SHOW_H__

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
class show : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x33;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    uint32_t                          oid;
    fb::model::point<uint16_t>        position;
    DIRECTION                         direction;
    HEAD_MARKER                       head_marker;
    std::string                       name;
    fb::game::character_appearance<V> appearance;
    CLIENT_UI_MODE                    ui_mode = CLIENT_UI_MODE::OLD;
#else
    uint16_t    x;
    uint16_t    y;
    uint8_t     direction;
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
    show(const fb::game::character& ch, const fb::game::object& to);
    show(uint32_t                                 oid,
         const fb::model::point<uint16_t>&        position,
         DIRECTION                                direction,
         HEAD_MARKER                              head_marker,
         std::string                              name,
         const fb::game::character_appearance<V>& appearance,
         CLIENT_UI_MODE                           ui_mode);
    show(const show&) = delete;
#else
    show() = default;
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
void show<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#else
template <>
void show<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

using show_v550 = show<CLIENT_VERSION::v550>;
using show_v565 = show<CLIENT_VERSION::v565>;
using show_v651 = show<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
