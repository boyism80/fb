#include <fb/game/context.h>

using namespace fb::game;

// new dialog
void context::on_dialog(character&               me,
                        const fb::model::object& object,
                        const std::string&       message,
                        bool                     button_prev,
                        bool                     button_next,
                        uint32_t                 sequence)
{
    me.send(fb_resp::dialog(object, message, button_prev, button_next, sequence));
}

void context::on_dialog(character&                      me,
                        const fb::model::object&        obj,
                        const std::string&              message,
                        const std::vector<std::string>& menus,
                        uint32_t                        sequence)
{
    me.send(fb_resp::dialog_menu(obj, menus, message, sequence));
}

void context::on_dialog(character&                      me,
                        const fb::model::object&        obj,
                        const std::string&              message,
                        const std::vector<std::string>& menus,
                        bool                            button_prev,
                        uint32_t                        sequence)
{
    me.send(fb_resp::dialog_list(obj, menus, message, button_prev, sequence));
}

void context::on_dialog(character&                      me,
                        const fb::model::object&        obj,
                        const std::string&              message,
                        const std::vector<std::string>& menus,
                        bool                            button_prev,
                        const dialog::preset&           preset,
                        uint32_t                        sequence)
{
    me.send(fb_resp::dialog_ch_list(obj, preset, menus, message, button_prev, sequence));
}

void context::on_dialog(character&                  me,
                        const fb::model::object&    obj,
                        const std::string&          message,
                        const std::vector<uint8_t>& item_slots,
                        uint32_t                    sequence)
{
    me.send(fb_resp::dialog_slot(obj, item_slots, message, sequence));
}

void context::on_dialog(character&                me,
                        const fb::model::object&  obj,
                        const std::string&        message,
                        const dialog::item_pairs& pairs,
                        uint32_t                  sequence,
                        uint16_t                  pursuit)
{
    me.send(fb_resp::dialog_item(obj, pairs, message, sequence, pursuit));
}

void context::on_dialog(character& me, const fb::model::object& obj, const std::string& message, uint32_t sequence)
{
    me.send(fb_resp::dialog_input(obj, message, sequence));
}

void context::on_dialog(character&               me,
                        const fb::model::object& obj,
                        const std::string&       message,
                        const std::string&       top,
                        const std::string&       bottom,
                        int                      maxlen,
                        bool                     prev,
                        uint32_t                 sequence)
{
    me.send(fb_resp::dialog_input_ext(obj, message, top, bottom, maxlen, prev, sequence));
}