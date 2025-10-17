#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stack>
#include <fb/game/npc.h>
#include <fb/game/portrait.h>

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

// clang-format off
struct listener_t
{
    virtual void on_dialog(character& me, const fb::model::object& object, const std::string& message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD)     = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, std::unique_ptr<portrait>&& portrait, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, uint32_t seq = 0xFFFFFFFD) = 0;
};
// clang-format on
} // namespace fb::game::dialog
#endif