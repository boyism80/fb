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


    // item
    virtual void on_remove_item(session& me, uint8_t index, item::delete_attr attr = item::delete_attr::DELETE_NONE) = 0;
    virtual void on_update_item(session& me, uint8_t index) = 0;
    virtual void on_active_item(session& me, item& item) = 0;

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
};


} }

#endif // !__LISTENER_H__
