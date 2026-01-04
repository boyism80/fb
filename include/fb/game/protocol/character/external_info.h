#ifndef __PROTOCOL_GAME_EXTERNAL_INFO_H__
#define __PROTOCOL_GAME_EXTERNAL_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class external_info : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x34;

#ifdef BOT

public:
    struct equipment_data
    {
        uint16_t look;
        uint8_t  color;
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

    // Equipment information
    uint8_t  armor_dress;
    uint8_t  armor_color;
    uint16_t weapon_dress;
    uint8_t  weapon_color;
    uint8_t  shield_dress;
    uint8_t  shield_color;

    // Additional equipment
    equipment_data helmet;
    equipment_data ring_left;
    equipment_data ring_right;
    equipment_data aux_left;
    equipment_data aux_right;

    // Other information
    std::string equipment_text;
    uint32_t    oid;
    uint8_t     group_option;
    uint8_t     trade_option;
    uint32_t    unknown;

    // Achievement information
    std::vector<achievement_data> achievements;
#endif

public:
#ifndef BOT
    external_info(const fb::game::character& ch);
#else
    external_info() = default;
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