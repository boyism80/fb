#include <fb/game/server.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_dialog(character&                 me,
                              std::optional<std::string> message,
                              bool                       button_prev,
                              bool                       button_next,
                              uint32_t                   oid)
{
    me.send(game_resp::dialog(std::move(message), button_prev, button_next, oid));
}

void listener_impl::on_dialog(character&                 me,
                              const fb::model::object&   object,
                              std::optional<std::string> message,
                              bool                       button_prev,
                              bool                       button_next,
                              uint32_t                   oid)
{
    me.send(game_resp::dialog(object, std::move(message), button_prev, button_next, oid));
}

void listener_impl::on_dialog(character&                 me,
                              const fb::game::object&    obj,
                              std::optional<std::string> message,
                              bool                       button_prev,
                              bool                       button_next,
                              uint32_t                   oid)
{
    me.send(game_resp::dialog(obj, std::move(message), button_prev, button_next, oid));
}

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::optional<std::string>              message,
                              bool                                    button_prev,
                              bool                                    button_next,
                              uint32_t                                oid)
{
    me.send(game_resp::dialog(std::move(appearance), std::move(message), button_prev, button_next, oid));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::model::object&        obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              uint32_t                        oid,
                              std::optional<std::string>      ext)
{
    me.send(game_resp::dialog_menu(obj, menus, message, oid, std::move(ext)));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::game::object&         obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              uint32_t                        oid,
                              std::optional<std::string>      ext)
{
    me.send(game_resp::dialog_menu(obj, menus, message, oid, std::move(ext)));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::model::object&        obj,
                              std::optional<std::string>      message,
                              const std::vector<std::string>& menus,
                              bool                            button_prev,
                              uint32_t                        oid)
{
    me.send(game_resp::dialog_list(obj, menus, std::move(message), button_prev, oid));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::game::object&         obj,
                              std::optional<std::string>      message,
                              const std::vector<std::string>& menus,
                              bool                            button_prev,
                              uint32_t                        oid)
{
    me.send(game_resp::dialog_list(obj, menus, std::move(message), button_prev, oid));
}

void listener_impl::on_dialog(character&                              me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::optional<std::string>              message,
                              const std::vector<std::string>&         menus,
                              bool                                    button_prev,
                              uint32_t                                oid)
{
    me.send(game_resp::dialog_list(std::move(appearance), menus, std::move(message), button_prev, oid));
}

void listener_impl::on_dialog(character&                  me,
                              const fb::model::object&    obj,
                              std::string_view            message,
                              const std::vector<uint8_t>& item_slots,
                              uint32_t                    oid)
{
    me.send(game_resp::dialog_slot(obj, item_slots, message, oid));
}

void listener_impl::on_dialog(character&                  me,
                              const fb::game::object&     obj,
                              std::string_view            message,
                              const std::vector<uint8_t>& item_slots,
                              uint32_t                    oid)
{
    me.send(game_resp::dialog_slot(obj, item_slots, message, oid));
}

void listener_impl::on_dialog(character&                me,
                              const fb::model::object&  obj,
                              std::string_view          message,
                              const dialog::item_pairs& pairs,
                              uint32_t                  oid,
                              uint16_t                  pursuit)
{
    me.send(game_resp::dialog_item(obj, pairs, message, oid, pursuit));
}

void listener_impl::on_dialog(character&                me,
                              const fb::game::object&   obj,
                              std::string_view          message,
                              const dialog::item_pairs& pairs,
                              uint32_t                  oid,
                              uint16_t                  pursuit)
{
    me.send(game_resp::dialog_item(obj, pairs, message, oid, pursuit));
}

void listener_impl::on_dialog(character&                 me,
                              const fb::model::object&   obj,
                              std::string_view           message,
                              uint32_t                   oid,
                              std::optional<std::string> ext,
                              uint16_t                   pursuit)
{
    me.send(game_resp::dialog_input(obj, message, oid, std::move(ext), pursuit));
}

void listener_impl::on_dialog(character&                 me,
                              const fb::game::object&    obj,
                              std::string_view           message,
                              uint32_t                   oid,
                              std::optional<std::string> ext,
                              uint16_t                   pursuit)
{
    me.send(game_resp::dialog_input(obj, message, oid, std::move(ext), pursuit));
}

void listener_impl::on_dialog(character&                 me,
                              const fb::model::object&   obj,
                              std::optional<std::string> message,
                              std::string_view           top,
                              std::string_view           bottom,
                              int                        maxlen,
                              bool                       prev,
                              bool                       password,
                              uint32_t                   oid)
{
    me.send(game_resp::dialog_input_ext(obj, std::move(message), top, bottom, maxlen, prev, password, oid));
}

void listener_impl::on_dialog(character&                 me,
                              const fb::game::object&    obj,
                              std::optional<std::string> message,
                              std::string_view           top,
                              std::string_view           bottom,
                              int                        maxlen,
                              bool                       prev,
                              bool                       password,
                              uint32_t                   oid)
{
    me.send(game_resp::dialog_input_ext(obj, std::move(message), top, bottom, maxlen, prev, password, oid));
}

void listener_impl::on_dialog_pursuit(character&                      me,
                                      const fb::model::object&        obj,
                                      std::string_view                message,
                                      const std::vector<std::string>& options,
                                      uint32_t                        oid,
                                      uint16_t                        pursuit)
{
    me.send(game_resp::dialog_pursuit(obj, options, message, oid, pursuit));
}

void listener_impl::on_dialog_pursuit(character&                      me,
                                      const fb::game::object&         obj,
                                      std::string_view                message,
                                      const std::vector<std::string>& options,
                                      uint32_t                        oid,
                                      uint16_t                        pursuit)
{
    me.send(game_resp::dialog_pursuit(obj, options, message, oid, pursuit));
}

void listener_impl::on_dialog_spell(character&               me,
                                    const fb::model::object& obj,
                                    std::string_view         message,
                                    uint32_t                 oid,
                                    uint16_t                 pursuit)
{
    me.send(game_resp::dialog_spell(obj, message, oid, pursuit));
}

void listener_impl::on_dialog_spell(character&              me,
                                    const fb::game::object& obj,
                                    std::string_view        message,
                                    uint32_t                oid,
                                    uint16_t                pursuit)
{
    me.send(game_resp::dialog_spell(obj, message, oid, pursuit));
}

void listener_impl::on_dialog_dual_field(character&                                              me,
                                         const fb::model::object&                                obj,
                                         std::string_view                                        message,
                                         const std::vector<std::pair<std::string, std::string>>& pairs,
                                         uint32_t                                                oid,
                                         uint16_t                                                pursuit)
{
    me.send(game_resp::dialog_dual_field(obj, pairs, message, oid, pursuit));
}

void listener_impl::on_dialog_dual_field(character&                                              me,
                                         const fb::game::object&                                 obj,
                                         std::string_view                                        message,
                                         const std::vector<std::pair<std::string, std::string>>& pairs,
                                         uint32_t                                                oid,
                                         uint16_t                                                pursuit)
{
    me.send(game_resp::dialog_dual_field(obj, pairs, message, oid, pursuit));
}

void listener_impl::on_dialog_email(character&               me,
                                    const fb::model::object& obj,
                                    std::string_view         message,
                                    std::string_view         str1,
                                    std::string_view         str2,
                                    bool                     button_prev,
                                    bool                     button_next,
                                    uint32_t                 oid)
{
    me.send(game_resp::dialog_email(obj, message, str1, str2, button_prev, button_next, oid));
}

void listener_impl::on_dialog_email(character&              me,
                                    const fb::game::object& obj,
                                    std::string_view        message,
                                    std::string_view        str1,
                                    std::string_view        str2,
                                    bool                    button_prev,
                                    bool                    button_next,
                                    uint32_t                oid)
{
    me.send(game_resp::dialog_email(obj, message, str1, str2, button_prev, button_next, oid));
}

void listener_impl::on_dialog_0x30_10(character&               me,
                                      const fb::model::object& obj,
                                      std::string_view         message,
                                      uint32_t                 oid)
{
    me.send(game_resp::dialog_0x30_10(obj, message, false, false, oid));
}

void listener_impl::on_dialog_0x30_10(character&              me,
                                      const fb::game::object& obj,
                                      std::string_view        message,
                                      uint32_t                oid)
{
    me.send(game_resp::dialog_0x30_10(obj, message, false, false, oid));
}
