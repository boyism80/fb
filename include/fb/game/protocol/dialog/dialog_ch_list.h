#ifndef __PROTOCOL_GAME_CH_LIST_H__
#define __PROTOCOL_GAME_CH_LIST_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class dialog_ch_list : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x30;

public:
#ifndef BOT
    const fb::model::object&            obj;
    const fb::game::dialog::preset&     preset;
    const std::vector<std::string>      list;
    const std::string                   message;
    const bool                          button_prev;
    const fb::game::dialog::interaction interaction;
    const uint32_t                      sequence;
#else
    uint8_t                  interaction;
    uint32_t                 sequence;
    uint8_t                  sex;
    uint8_t                  state;
    uint16_t                 face;
    uint8_t                  hair_color;
    uint8_t                  armor;
    uint8_t                  armor_color;
    uint16_t                 weapon;
    uint8_t                  weapon_color;
    uint16_t                 shield;
    uint8_t                  shield_color;
    std::string              message;
    std::vector<std::string> list;
#endif

public:
#ifndef BOT
    dialog_ch_list(const fb::model::object&        obj,
                   const fb::game::dialog::preset& preset,
                   const std::vector<std::string>& list,
                   const std::string&              message,
                   bool                            button_prev,
                   uint32_t                        sequence    = 0xFFFFFFFD,
                   fb::game::dialog::interaction   interaction = fb::game::dialog::interaction::LIST);
#else
    dialog_ch_list() = default;
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