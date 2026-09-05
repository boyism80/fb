#ifndef __PROTOCOL_GAME_EXTERNAL_INFO_H__
#define __PROTOCOL_GAME_EXTERNAL_INFO_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <vector>
#ifndef BOT
#include <fb/game/character.h>
#include <fb/game/appearance.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class external_info : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x34;
    FB_PROTOCOL_VERSION_TAGS(V);
#ifdef BOT

public:
    struct equipment_data
    {
        uint16_t    look  = 0;
        uint8_t     color = 0;
        std::string name_a;
        std::string unknown_name_b;
        uint32_t    unknown_u32 = 0;
    };

    struct achievement_data
    {
        uint8_t     icon;
        uint8_t     color;
        std::string text;
    };
#endif

public:
#ifndef BOT
    const fb::game::character& ch;
    const fb::game::object&    to;
#else
    std::string title;
    std::string clan_name;
    std::string clan_title;
    std::string class_name;
    std::string name;
    uint8_t     disguised;
    uint8_t     gender;
    uint8_t     state;
    uint16_t    look;
    uint8_t     color;

    uint8_t  armor_dress;
    uint8_t  armor_color;
    uint16_t weapon_dress;
    uint8_t  weapon_color;
    uint8_t  shield_dress;
    uint8_t  shield_color;

    equipment_data helmet;
    equipment_data ring_left;
    equipment_data ring_right;
    equipment_data aux_left;
    equipment_data aux_right;

    std::string equipment_text;
    uint32_t    oid;
    uint8_t     group_option;
    uint8_t     trade_option;
    int16_t     reputation = 0; // 인품, oid trailer u16be → client this+0xB64

    std::vector<achievement_data> achievements;
#endif

public:
#ifndef BOT
    external_info(const fb::game::character& ch, const fb::game::object& to);
#else
    external_info() = default;
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
void external_info<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#else
template <>
void external_info<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

using external_info_v550 = external_info<CLIENT_VERSION::v550>;
using external_info_v565 = external_info<CLIENT_VERSION::v565>;
using external_info_v651 = external_info<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
