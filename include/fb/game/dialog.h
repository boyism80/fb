#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stack>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <vector>
#include <fb/game/dialog_type.h>
#include <fb/game/npc.h>
#include <fb/game/appearance.h>

namespace fb::game {
class character;
}

namespace fb::game::dialog {
using item_pair  = std::pair<fb::model::item&, uint32_t>;
using item_pairs = std::vector<item_pair>;

// clang-format off
struct listener_t
{
    virtual void on_dialog(character& me, std::optional<std::string> message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& object, std::optional<std::string> message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, std::optional<std::string> message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, std::string_view message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD, std::optional<std::string> ext = std::nullopt) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, std::string_view message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD, std::optional<std::string> ext = std::nullopt) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, std::optional<std::string> message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, std::optional<std::string> message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, std::unique_ptr<appearance>&& appearance_ptr, std::optional<std::string> message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, std::string_view message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, std::string_view message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, std::string_view message, const item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, std::string_view message, const item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD, std::optional<std::string> ext = std::nullopt, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD, std::optional<std::string> ext = std::nullopt, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, std::optional<std::string> message, std::string_view top, std::string_view bottom, int maxlen = 0xFF, bool prev = false, bool password = false, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, std::optional<std::string> message, std::string_view top, std::string_view bottom, int maxlen = 0xFF, bool prev = false, bool password = false, uint32_t seq = 0xFFFFFFFD) = 0;

    virtual void on_dialog_pursuit(character& me, const fb::model::object& obj, std::string_view message, const std::vector<std::string>& options, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog_pursuit(character& me, const fb::game::object& obj, std::string_view message, const std::vector<std::string>& options, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog_spell(character& me, const fb::model::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog_spell(character& me, const fb::game::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog_dual_field(character& me, const fb::model::object& obj, std::string_view message, const std::vector<std::pair<std::string, std::string>>& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog_dual_field(character& me, const fb::game::object& obj, std::string_view message, const std::vector<std::pair<std::string, std::string>>& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;

    virtual void on_dialog_email(character& me, const fb::model::object& obj, std::string_view message, std::string_view str1, std::string_view str2, bool button_prev = false, bool button_next = false, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog_email(character& me, const fb::game::object& obj, std::string_view message, std::string_view str1, std::string_view str2, bool button_prev = false, bool button_next = false, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog_0x30_10(character& me, const fb::model::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog_0x30_10(character& me, const fb::game::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD) = 0;
};
// clang-format on
} // namespace fb::game::dialog
#endif
