#ifndef __PROTOCOL_GAME_INTERNAL_INFO_H__
#define __PROTOCOL_GAME_INTERNAL_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class internal_info : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x39;

public:
#ifndef BOT
    fb::game::character&    ch;
    const fb::model::model& model;
#else
    struct equipment_data
    {
        uint16_t look;
        uint8_t  color;
    };

    struct achievement_data
    {
        uint8_t     look;
        uint8_t     color;
        std::string text;
    };

    int8_t                        phydef;
    uint8_t                       dam;
    uint8_t                       hit;
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
    internal_info(fb::game::character& ch, const fb::model::model& model) :
        ch(ch),
        model(model)
    { }
#else
    internal_info() = default;
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