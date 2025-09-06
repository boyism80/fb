#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stack>
#include <fb/game/npc.h>

namespace fb::game {
class character;
}

namespace fb::game::dialog {
using item_pair  = std::pair<fb::model::item&, uint32_t>;
using item_pairs = std::vector<item_pair>;

enum class interaction : uint8_t
{
    NORMAL,
    INPUT,
    INPUT_EX,
    MENU,
    LIST,
    SLOT,
    ITEM,
};

struct preset
{
    SEX                     sex          = SEX::MAN;
    STATE                   state        = STATE::NORMAL;
    uint16_t                face         = 0;
    uint8_t                 hair_color   = 0;
    std::optional<uint16_t> weapon       = std::nullopt;
    std::optional<uint8_t>  weapon_color = std::nullopt;
    std::optional<uint8_t>  armor        = std::nullopt;
    std::optional<uint8_t>  armor_color  = std::nullopt;
    std::optional<uint16_t> shield       = std::nullopt;
    std::optional<uint8_t>  shield_color = std::nullopt;

    preset() = default;

    preset(const character& ch);
};

struct listener_t
{
    virtual void on_dialog(character& me, const fb::model::object& object, const std::string& message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD) = 0;

    virtual void
    on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;

    virtual void on_dialog(character&                      me,
                           const fb::model::object&        obj,
                           const std::string&              message,
                           const std::vector<std::string>& menus,
                           bool                            button_prev,
                           const dialog::preset&           preset,
                           uint32_t                        seq = 0xFFFFFFFD)                                                                                                                  = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) = 0;

    virtual void
    on_dialog(character& me, const fb::model::object& obj, const std::string& message, const item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, uint32_t seq = 0xFFFFFFFD)                                        = 0;

    virtual void on_dialog(character&               me,
                           const fb::model::object& obj,
                           const std::string&       message,
                           const std::string&       top,
                           const std::string&       bottom,
                           int                      maxlen = 0xFF,
                           bool                     prev   = false,
                           uint32_t                 seq    = 0xFFFFFFFD) = 0;
};
} // namespace fb::game::dialog
#endif