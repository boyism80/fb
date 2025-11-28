#include <fb/game/server.h>

using namespace fb::game;

void listener_impl::on_dialog(character& me, const fb::model::object& object, const std::string& message, bool button_prev, bool button_next, uint32_t oid)
{
    me.send(fb_resp::dialog(object, message, button_prev, button_next, oid));
}

void listener_impl::on_dialog(character& me, const fb::game::object& obj, const std::string& message, bool button_prev, bool button_next, uint32_t oid)
{
    me.send(fb_resp::dialog(obj, message, button_prev, button_next, oid));
}

void listener_impl::on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, uint32_t oid)
{
    me.send(fb_resp::dialog_menu(obj, menus, message, oid));
}

void listener_impl::on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<std::string>& menus, uint32_t oid)
{
    me.send(fb_resp::dialog_menu(obj, menus, message, oid));
}

void listener_impl::on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t oid)
{
    me.send(fb_resp::dialog_list(obj, menus, message, button_prev, oid));
}

void listener_impl::on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t oid)
{
    me.send(fb_resp::dialog_list(obj, menus, message, button_prev, oid));
}

void listener_impl::on_dialog(character&                            me,
                              std::unique_ptr<fb::game::portrait>&& portrait,
                              const std::string&                    message,
                              const std::vector<std::string>&       menus,
                              bool                                  button_prev,
                              uint32_t                              oid)
{
    me.send(fb_resp::dialog_list(std::move(portrait), menus, message, button_prev, oid));
}

void listener_impl::on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<uint8_t>& item_slots, uint32_t oid)
{
    me.send(fb_resp::dialog_slot(obj, item_slots, message, oid));
}

void listener_impl::on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<uint8_t>& item_slots, uint32_t oid)
{
    me.send(fb_resp::dialog_slot(obj, item_slots, message, oid));
}

void listener_impl::on_dialog(character& me, const fb::model::object& obj, const std::string& message, const dialog::item_pairs& pairs, uint32_t oid, uint16_t pursuit)
{
    me.send(fb_resp::dialog_item(obj, pairs, message, oid, pursuit));
}

void listener_impl::on_dialog(character& me, const fb::game::object& obj, const std::string& message, const dialog::item_pairs& pairs, uint32_t oid, uint16_t pursuit)
{
    me.send(fb_resp::dialog_item(obj, pairs, message, oid, pursuit));
}

void listener_impl::on_dialog(character& me, const fb::model::object& obj, const std::string& message, uint32_t oid)
{
    me.send(fb_resp::dialog_input(obj, message, oid));
}

void listener_impl::on_dialog(character& me, const fb::game::object& obj, const std::string& message, uint32_t oid)
{
    me.send(fb_resp::dialog_input(obj, message, oid));
}

void listener_impl::on_dialog(character&               me,
                              const fb::model::object& obj,
                              const std::string&       message,
                              const std::string&       top,
                              const std::string&       bottom,
                              int                      maxlen,
                              bool                     prev,
                              uint32_t                 oid)
{
    me.send(fb_resp::dialog_input_ext(obj, message, top, bottom, maxlen, prev, oid));
}

void listener_impl::on_dialog(character&              me,
                              const fb::game::object& obj,
                              const std::string&      message,
                              const std::string&      top,
                              const std::string&      bottom,
                              int                     maxlen,
                              bool                    prev,
                              uint32_t                oid)
{
    me.send(fb_resp::dialog_input_ext(obj, message, top, bottom, maxlen, prev, oid));
}