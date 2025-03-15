#include <fb/game/context.h>

using namespace fb::game;

// new dialog
void context::on_dialog(character&               me,
                        const fb::model::object& object,
                        const std::string&       message,
                        bool                     button_prev,
                        bool                     button_next)
{
    me.send(fb_resp::dialog(object, message, button_prev, button_next));
}

void context::on_dialog(character&                      me,
                        const fb::model::object&        obj,
                        const std::string&              message,
                        const std::vector<std::string>& menus)
{
    me.send(fb_resp::dialog_menu(obj, menus, message));
}

void context::on_dialog(character&                      me,
                        const fb::model::object&        obj,
                        const std::string&              message,
                        const std::vector<std::string>& menus,
                        bool                            button_prev)
{
    me.send(fb_resp::dialog_list(obj, menus, message, button_prev));
}

void context::on_dialog(character&                      me,
                        const fb::model::object&        obj,
                        const std::string&              message,
                        const std::vector<std::string>& menus,
                        bool                            button_prev,
                        const dialog::preset&           preset)
{
    me.send(fb_resp::dialog_ch_list(obj, preset, menus, message, button_prev));
}

void context::on_dialog(character&                  me,
                        const fb::model::object&    obj,
                        const std::string&          message,
                        const std::vector<uint8_t>& item_slots)
{
    me.send(fb_resp::dialog_slot(obj, item_slots, message));
}

void context::on_dialog(character&                me,
                        const fb::model::object&  obj,
                        const std::string&        message,
                        const dialog::item_pairs& pairs,
                        uint16_t                  pursuit)
{
    me.send(fb_resp::dialog_item(obj, pairs, message, pursuit));
}

void context::on_dialog(character& me, const fb::model::object& obj, const std::string& message)
{
    me.send(fb_resp::dialog_input(obj, message));
}

void context::on_dialog(character&               me,
                        const fb::model::object& obj,
                        const std::string&       message,
                        const std::string&       top,
                        const std::string&       bottom,
                        int                      maxlen,
                        bool                     prev)
{
    me.send(fb_resp::dialog_input_ext(obj, message, top, bottom, maxlen, prev));
}