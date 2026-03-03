#include <fb/game/server.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_dialog(character& me, std::string_view message, bool button_prev, bool button_next, uint32_t oid)
{
    me.send(game_resp::dialog(message, button_prev, button_next, oid));
}

void listener_impl::on_dialog(character&               me,
                              const fb::model::object& object,
                              std::string_view         message,
                              bool                     button_prev,
                              bool                     button_next,
                              uint32_t                 oid)
{
    me.send(game_resp::dialog(object, message, button_prev, button_next, oid));
}

void listener_impl::on_dialog(character&              me,
                              const fb::game::object& obj,
                              std::string_view        message,
                              bool                    button_prev,
                              bool                    button_next,
                              uint32_t                oid)
{
    me.send(game_resp::dialog(obj, message, button_prev, button_next, oid));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::model::object&        obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              uint32_t                        oid)
{
    me.send(game_resp::dialog_menu(obj, menus, message, oid));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::game::object&         obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              uint32_t                        oid)
{
    me.send(game_resp::dialog_menu(obj, menus, message, oid));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::model::object&        obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              bool                            button_prev,
                              uint32_t                        oid)
{
    me.send(game_resp::dialog_list(obj, menus, std::string(message), button_prev, oid));
}

void listener_impl::on_dialog(character&                      me,
                              const fb::game::object&         obj,
                              std::string_view                message,
                              const std::vector<std::string>& menus,
                              bool                            button_prev,
                              uint32_t                        oid)
{
    me.send(game_resp::dialog_list(obj, menus, std::string(message), button_prev, oid));
}

void listener_impl::on_dialog(character&                            me,
                              std::unique_ptr<fb::game::appearance>&& appearance,
                              std::string_view                      message,
                              const std::vector<std::string>&       menus,
                              bool                                  button_prev,
                              uint32_t                              oid)
{
    me.send(game_resp::dialog_list(std::move(appearance), menus, message, button_prev, oid));
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

void listener_impl::on_dialog(character& me, const fb::model::object& obj, std::string_view message, uint32_t oid)
{
    me.send(game_resp::dialog_input(obj, message, oid));
}

void listener_impl::on_dialog(character& me, const fb::game::object& obj, std::string_view message, uint32_t oid)
{
    me.send(game_resp::dialog_input(obj, message, oid));
}

void listener_impl::on_dialog(character&               me,
                              const fb::model::object& obj,
                              std::string_view         message,
                              std::string_view         top,
                              std::string_view         bottom,
                              int                      maxlen,
                              bool                     prev,
                              uint32_t                 oid)
{
    me.send(game_resp::dialog_input_ext(obj, message, top, bottom, maxlen, prev, oid));
}

void listener_impl::on_dialog(character&              me,
                              const fb::game::object& obj,
                              std::string_view        message,
                              std::string_view        top,
                              std::string_view        bottom,
                              int                     maxlen,
                              bool                    prev,
                              uint32_t                oid)
{
    me.send(game_resp::dialog_input_ext(obj, message, top, bottom, maxlen, prev, oid));
}