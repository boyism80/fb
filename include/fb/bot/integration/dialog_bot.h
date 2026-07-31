#ifndef __BOT_INTEGRATION_DIALOG_BOT_H__
#define __BOT_INTEGRATION_DIALOG_BOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/game/dialog_type.h>
#include <utility>
#include <vector>
#include <string>

namespace fb::bot::integration {

// Wire values match fb::game::dialog::type (opcode 0x2F).
enum class dialog_type : uint8_t
{
    menu_no_ext  = static_cast<uint8_t>(fb::game::dialog::type::MENU_NO_EXT),
    menu         = static_cast<uint8_t>(fb::game::dialog::type::MENU),
    input_no_ext = static_cast<uint8_t>(fb::game::dialog::type::INPUT_NO_EXT),
    input        = static_cast<uint8_t>(fb::game::dialog::type::INPUT),
    item         = static_cast<uint8_t>(fb::game::dialog::type::ITEM),
    slot         = static_cast<uint8_t>(fb::game::dialog::type::SLOT),
    pursuit      = static_cast<uint8_t>(fb::game::dialog::type::PURSUIT),
    spell        = static_cast<uint8_t>(fb::game::dialog::type::SPELL),
    dual_field   = static_cast<uint8_t>(fb::game::dialog::type::DUAL_FIELD),
};

struct dialog_item_data
{
    uint16_t    look;
    uint8_t     color;
    uint32_t    value;
    std::string name;
    std::string desc;
};

class dialog_bot : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2F;

public:
    dialog_type type = dialog_type::menu_no_ext;

    uint16_t    look  = 0;
    uint8_t     color = 0;
    std::string message;
    uint32_t    oid         = 0;
    uint8_t     interaction = 0;
    uint16_t    pursuit     = 0xFFFF;

    std::vector<dialog_item_data>                    item_items;
    uint16_t                                         item_pursuit = 0xFFFF;
    std::vector<std::string>                         menu_menus;
    std::vector<uint8_t>                             slot_slots;
    std::vector<std::pair<std::string, std::string>> dual_pairs;

public:
    dialog_bot() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::bot::integration

#endif
