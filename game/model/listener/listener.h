#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <model/item/item.h>
#include <model/session/session.h>
#include <model/mob/mob.h>

namespace fb { namespace game {

class listener
{
public:
    // object
    virtual void on_direction(fb::game::object& me) = 0;
    virtual void on_show(fb::game::object& me, bool light) = 0;
    virtual void on_show(fb::game::object& me, fb::game::object& you, bool light) = 0;
    virtual void on_hide(fb::game::object& me) = 0;
    virtual void on_hide(fb::game::object& me, fb::game::object& you) = 0;
    virtual void on_move(fb::game::object& me) = 0;
    virtual void on_warp(fb::game::object& me) = 0;
    virtual void on_unbuff(fb::game::object& me, fb::game::buff& buff) = 0;


    // life
    virtual void on_attack(life& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void on_damage(life& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void on_heal_hp(life& me, uint32_t value, fb::game::object* from) = 0;
    virtual void on_heal_mp(life& me, uint32_t value, fb::game::object* from) = 0;
    virtual void on_die(life& me) = 0;
    virtual void on_hp(life& me, uint32_t before, uint32_t current) = 0;
    virtual void on_mp(life& me, uint32_t before, uint32_t current) = 0;

    // mob
    virtual void on_attack(mob& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void on_damage(mob& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void on_die(mob& me) = 0;


    // session
    virtual void on_attack(session& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void on_damage(session& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void on_hold(session& me) = 0;
    virtual void on_die(session& me) = 0;
    virtual void on_action(session& me, fb::game::action action, duration duration, uint8_t sound) = 0;
    virtual void on_updated(session& me, state_level level = state_level::LEVEL_MIN) = 0;
    virtual void on_notify(session& me, const std::string& message, message::type type = message::type::STATE) = 0;
    virtual void on_option(session& me, fb::game::options option, bool enabled) = 0;
    virtual void on_level_up(session& me) = 0;
    virtual void on_warp(session& me, fb::game::map& map, const point16_t& position) = 0;
    virtual void on_item_get(session& me, fb::game::item& item, uint8_t slot) = 0;
    virtual void on_item_changed(session& me, fb::game::item& item, uint8_t slot) = 0;
    virtual void on_item_lost(session& me, uint8_t slot) = 0;


    // item
    virtual void on_item_remove(session& me, uint8_t index, item::delete_attr attr = item::delete_attr::DELETE_NONE) = 0;
    virtual void on_item_update(session& me, uint8_t index) = 0;
    virtual void on_item_active(session& me, item& item) = 0;
    virtual void on_item_throws(session& me, item& item, const point16_t& to) = 0;

    
    // spell
    virtual void on_spell_update(life& me, uint8_t index) = 0;
    virtual void on_spell_remove(life& me, uint8_t index) = 0;


    // equipment
    virtual void on_equipment_on(session& me, item& item, equipment::slot slot) = 0;
    virtual void on_equipment_off(session& me, equipment::slot slot) = 0;


    // trade
    virtual void on_trade_begin(session& me, session& you) = 0;
    virtual void on_trade_bundle(session& me) = 0;
    virtual void on_trade_item(session& me, session& from, uint8_t index) = 0;
    virtual void on_trade_money(session& me, session& from) = 0;
    virtual void on_trade_cancel(session& me) = 0;
    virtual void on_trade_lock(session& me, bool mine) = 0;
    virtual void on_trade_failed(session& me) = 0;
    virtual void on_trade_success(session& me) = 0;

    // dialog
    virtual void on_dialog(session& me, const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message,  fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    virtual void on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;

    // game
    virtual void on_save(session& me) = 0;
};


} }

#endif // !__LISTENER_H__
