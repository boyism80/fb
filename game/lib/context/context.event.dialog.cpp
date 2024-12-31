#include <fb/game/context.h>

using namespace fb::game;

// new dialog
void context::on_dialog(character&               me,
                        const fb::model::object& object,
                        const std::string&       message,
                        bool                     button_prev,
                        bool                     button_next,
                        dialog::interaction      interaction)
{
    this->send(me, fb_resp::dialog(object, message, button_prev, button_next, interaction), scope::SELF);
}

void context::on_dialog(character&                      me,
                        const fb::model::npc&           npc,
                        const std::string&              message,
                        const std::vector<std::string>& menus,
                        dialog::interaction             interaction)
{
    this->send(me, fb_resp::dialog_menu(npc, menus, message, interaction), scope::SELF);
}

void context::on_dialog(character&                  me,
                        const fb::model::npc&       npc,
                        const std::string&          message,
                        const std::vector<uint8_t>& item_slots,
                        dialog::interaction         interaction)
{
    this->send(me, fb_resp::dialog_slot(npc, item_slots, message, interaction), scope::SELF);
}

void context::on_dialog(character&                me,
                        const fb::model::npc&     npc,
                        const std::string&        message,
                        const dialog::item_pairs& pairs,
                        uint16_t                  pursuit,
                        dialog::interaction       interaction)
{
    this->send(me, fb_resp::dialog_item(npc, pairs, message, pursuit, interaction), scope::SELF);
}

void context::on_dialog(character&            me,
                        const fb::model::npc& npc,
                        const std::string&    message,
                        dialog::interaction   interaction)
{
    this->send(me, fb_resp::dialog_input(npc, message, interaction), scope::SELF);
}

void context::on_dialog(character&            me,
                        const fb::model::npc& npc,
                        const std::string&    message,
                        const std::string&    top,
                        const std::string&    bottom,
                        int                   maxlen,
                        bool                  prev,
                        dialog::interaction   interaction)
{
    this->send(me, fb_resp::dialog_input_ext(npc, message, top, bottom, maxlen, prev, interaction), scope::SELF);
}