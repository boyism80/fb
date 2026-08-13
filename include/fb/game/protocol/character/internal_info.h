#ifndef __PROTOCOL_GAME_INTERNAL_INFO_H__
#define __PROTOCOL_GAME_INTERNAL_INFO_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class internal_info : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x39;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    fb::game::character& ch;
#else
    struct equipment_data
    {
        uint16_t    look;
        uint8_t     color;
        std::string name_a;
        std::string unknown_name_b;
        uint32_t    unknown_u32;
    };

    struct achievement_data
    {
        uint8_t     look;
        uint8_t     color;
        std::string text;
    };

    int8_t                        phydef;
    int8_t                        dam;
    int8_t                        hit;
    std::string                   clan_name;
    std::string                   clan_title;
    std::string                   title;
    std::string                   group_info;
    uint8_t                       group_option;
    uint32_t                      remained_exp;
    std::string                   class_name;
    std::vector<equipment_data>   equipments;
    uint8_t                       trade_option;
    uint8_t                       pk_protect_option;
    std::vector<achievement_data> achievements;
#endif

public:
#ifndef BOT
    internal_info(fb::game::character& ch) :
        ch(ch)
    { }
#else
    internal_info() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using internal_info_v550 = internal_info<CLIENT_VERSION::v550>;
using internal_info_v565 = internal_info<CLIENT_VERSION::v565>;
using internal_info_v651 = internal_info<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
