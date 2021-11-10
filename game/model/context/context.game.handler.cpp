#include "context.game.h"

void fb::game::context::on_create(fb::game::object& me)
{
    auto gd = std::lock_guard(this->_hash_mutex);

    if(this->_objects.contains(&me) == false)
        this->_objects.insert(std::make_pair(&me, std::unique_ptr<fb::game::object>(&me)));
}

void fb::game::context::on_destroy(fb::game::object& me)
{
    
}

void fb::game::context::on_direction(fb::game::object& me)
{
    this->send(me, fb::protocol::game::response::object::direction(me), scope::PIVOT, true);
}

void fb::game::context::on_show(fb::game::object& me, bool light)
{
    if(me.is(fb::game::object::types::SESSION))
    {
        this->send
        (
            me, 
            [&me, light](const fb::game::object& to)
            {
                return std::unique_ptr<fb::protocol::base::header>(new fb::protocol::game::response::session::show(static_cast<fb::game::session&>(me), to, light));
            }, 
            scope::PIVOT
        );
    }
    else
    {
        this->send(me, fb::protocol::game::response::object::show(me), scope::PIVOT);
    }
}

void fb::game::context::on_show(fb::game::object& me, fb::game::object& you, bool light)
{
    if(you.is(fb::game::object::types::SESSION))
        this->send(me, fb::protocol::game::response::session::show(static_cast<fb::game::session&>(you), me, light), scope::SELF);
    else
        this->send(me, fb::protocol::game::response::object::show(you), scope::SELF);
}

void fb::game::context::on_hide(fb::game::object& me)
{
    this->send(me, fb::protocol::game::response::object::hide(me), scope::PIVOT);
}

void fb::game::context::on_hide(fb::game::object& me, fb::game::object& you)
{
    this->send(me, fb::protocol::game::response::object::hide(you), scope::SELF);
}

void fb::game::context::on_move(fb::game::object& me)
{
    this->send(me, fb::protocol::game::response::object::move(me), scope::PIVOT, true);
}

void fb::game::context::on_unbuff(fb::game::object& me, fb::game::buff& buff)
{
    auto& uncast = buff.spell().uncast();
    if(uncast.empty())
        return;

     lua::get()
         .from(uncast.c_str())
         .func("on_uncast")
         .pushobject(me)
         .pushobject(buff.spell())
         .resume(2);
    this->send(me, fb::protocol::game::response::spell::unbuff(buff), scope::SELF);
}

void fb::game::context::on_enter(fb::game::object& me, fb::game::map& map, const point16_t& position)
{
    auto thread = this->thread(map);
    if(thread == nullptr || thread == this->threads().current())
    {
        me.handle_enter(map, position);

        if(me.is(fb::game::object::types::SESSION))
            this->save(static_cast<fb::game::session&>(me));
    }
    else
    {
        thread->precedence.enqueue
        (
            [this, &me, &map, position] (uint8_t) 
            {
                me.handle_enter(map, position);
                if(me.is(fb::game::object::types::SESSION))
                    this->save(static_cast<fb::game::session&>(me));
            }
        );
    }
}

void fb::game::context::on_attack(life& me, object* you)
{ }

void fb::game::context::on_hit(life& me, life& you, uint32_t damage, bool critical)
{ }

void fb::game::context::on_kill(life& me, life& you)
{ }

void fb::game::context::on_damaged(life& me, object* you, uint32_t damage, bool critical)
{
    this->send(me, fb::protocol::game::response::life::show_hp(me, damage, false), scope::PIVOT);
}

void fb::game::context::on_die(life& me, object* you)
{ }

void fb::game::context::on_heal_hp(life& me, uint32_t value, fb::game::object* from)
{ }

void fb::game::context::on_heal_mp(life& me, uint32_t value, fb::game::object* from)
{ }

void fb::game::context::on_hp(life& me, uint32_t before, uint32_t current)
{ }

void fb::game::context::on_mp(life& me, uint32_t before, uint32_t current)
{ }

void fb::game::context::on_action(session& me, action action, duration duration, uint8_t sound)
{
    this->send(me, fb::protocol::game::response::session::action(me, action, duration), scope::PIVOT);
}

void fb::game::context::on_updated(session& me, fb::game::state_level level)
{
    this->send(me, fb::protocol::game::response::session::state(me, level), scope::SELF);
}

void fb::game::context::on_money_changed(session& me, uint32_t value)
{ }

void fb::game::context::on_attack(session& me, object* you)
{
    this->send(me, fb::protocol::game::response::session::action(me, action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT);
    auto* weapon = me.items.weapon();
    if (weapon != nullptr)
    {
        auto            sound = weapon->based<fb::game::weapon>()->sound;
        this->send(me, fb::protocol::game::response::object::sound(me, sound != 0 ? fb::game::sound::type(sound) : fb::game::sound::type
::SWING), scope::PIVOT);
    }
}

void fb::game::context::on_hit(session& me, life& you, uint32_t damage, bool critical)
{
#ifndef PK
    if(you.is(object::types::SESSION))
        return;
#endif

    auto* weapon = me.items.weapon();
    if (weapon != nullptr)
        this->send(me, fb::protocol::game::response::object::sound(me, sound::type::DAMAGE), scope::PIVOT);

    you.hp_down(damage, &me, critical);
}

void fb::game::context::on_kill(session& me, life& you)
{ }

void fb::game::context::on_damaged(session& me, object* you, uint32_t damage, bool critical)
{ }

void fb::game::context::on_hold(session& me)
{
    this->send(me, fb::protocol::game::response::session::position(me), scope::SELF);
}

void fb::game::context::on_die(session& me, object* you)
{ }

void fb::game::context::on_notify(session& me, const std::string& message, fb::game::message::type type)
{
    this->send(me, fb::protocol::game::response::message(message, type), scope::SELF);
}

void fb::game::context::on_equipment_on(session& me, item& item, equipment::slot slot)
{
    this->send(me, fb::protocol::game::response::item::update_slot(me, slot), scope::SELF);
    this->send(me, fb::protocol::game::response::object::sound(me, sound::type::EQUIPMENT_ON), scope::PIVOT);

    std::stringstream sstream;
    switch(slot)
    {
    case fb::game::equipment::slot::WEAPON_SLOT:
        sstream << "w:무기  :";
        break;

    case fb::game::equipment::slot::ARMOR_SLOT:
        sstream << "a:갑옷  :";
        break;

    case fb::game::equipment::slot::SHIELD_SLOT:
        sstream << "s:방패  :";
        break;

    case fb::game::equipment::slot::HELMET_SLOT:
        sstream << "h:머리  :";
        break;

    case fb::game::equipment::slot::LEFT_HAND_SLOT:
        sstream << "l:왼손  :";
        break;

    case fb::game::equipment::slot::RIGHT_HAND_SLOT:
        sstream << "r:오른손  :";
        break;

    case fb::game::equipment::slot::LEFT_AUX_SLOT:
        sstream << "[:보조1  :";
        break;

    case fb::game::equipment::slot::RIGHT_AUX_SLOT:
        sstream << "]:보조2  :";
        break;
    }

    sstream << item.name();
    this->send(me, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::SELF);

    sstream.str(std::string());
    sstream << "갑옷 강도  " << me.defensive_physical() <<"  " << me.regenerative() << " S  " << me.defensive_magical();
    this->send(me, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::SELF);
}

void fb::game::context::on_equipment_off(session& me, equipment::slot slot, uint8_t index)
{
    this->send(me, fb::protocol::game::response::object::sound(me, sound::type::EQUIPMENT_OFF), scope::PIVOT);
}

void fb::game::context::on_item_active(session& me, item& item)
{
    lua::get()
        .from(item.based<fb::game::item>()->active_script.c_str())
        .func("on_active")
        .pushobject(me)
        .pushobject(item)
        .resume(2);
}

void fb::game::context::on_item_throws(session& me, item& item, const point16_t& to)
{
    if(me.position() != to)
        this->send(me, fb::protocol::game::response::session::throws(me, item, to), scope::PIVOT);
    else
        this->send(me, fb::protocol::game::response::session::action(me, fb::game::action::ATTACK, fb::game::duration::DURATION_THROW), scope::PIVOT);
}

void fb::game::context::on_spell_update(life& me, uint8_t index)
{
    this->send(me, fb::protocol::game::response::spell::update(me, index), scope::SELF);
}

void fb::game::context::on_spell_remove(life& me, uint8_t index)
{
    this->send(me, fb::protocol::game::response::spell::remove(me, index), scope::SELF);
}

void fb::game::context::on_trade_begin(session& me, session& you)
{
    this->send(me, fb::protocol::game::response::trade::dialog(you), scope::SELF);
}

void fb::game::context::on_trade_bundle(session& me)
{
    this->send(me, fb::protocol::game::response::trade::bundle(), scope::SELF);
}

void fb::game::context::on_trade_money(session& me, session& from)
{
    bool mine = (&me == &from);
    this->send(me, fb::protocol::game::response::trade::money(from, mine), scope::SELF);
}

void fb::game::context::on_trade_cancel(session& me, session& from)
{
    bool mine = (&me == &from);
    this->send(me, fb::protocol::game::response::trade::close(mine ? fb::game::message::trade::CANCELLED_BY_ME : fb::game::message::trade::CANCELLED_BY_PARTNER), scope::SELF);
}

void fb::game::context::on_trade_lock(session& me, bool mine)
{
    if(mine)
    {
        this->send(me, fb::protocol::game::response::trade::lock(), scope::SELF);
    }
    else
    {
        this->send(me, fb::protocol::game::response::message(fb::game::message::trade::NOTIFY_LOCK_TO_PARTNER, message::type::POPUP), scope::SELF);
    }
}

void fb::game::context::on_trade_failed(session& me)
{
    this->send(me, fb::protocol::game::response::trade::close(fb::game::message::trade::FAILED), scope::SELF);
}

void fb::game::context::on_trade_success(session& me)
{
    this->send(me, fb::protocol::game::response::trade::close(fb::game::message::trade::SUCCESS), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::common(object, message, button_prev, button_next, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::menu(npc, menus, message, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::slot(npc, item_slots, message, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::item(npc, cores, message, 0xFFFF, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message,  fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::input(npc, message, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::input_ext(npc, message, top, bottom, maxlen, prev, interaction), scope::SELF);
}

void fb::game::context::on_trade_item(session& me, session& from, uint8_t index)
{
    bool mine = (&me == &from);
    this->send(me, fb::protocol::game::response::trade::upload(from, index, mine), scope::SELF);
}

void fb::game::context::on_option(session& me, fb::game::options option, bool enabled)
{
    std::stringstream sstream;

    switch(option)
    {
    case options::WHISPER:
        sstream << "귓속말듣기  ";
        break;

    case options::GROUP:
    {
        auto group = me.group();
        if(group != nullptr)
        {
            if(group->members().size() == 1)
            {
                this->send(me, fb::protocol::game::response::message("그룹 해체", message::type::STATE), scope::GROUP);
                fb::game::group::destroy(*group);
            }
            else
            {
                auto leader = group->leave(me);
                std::stringstream sstream;
                sstream << me.name() << "님 그룹 탈퇴";
                this->send(*leader, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::GROUP);
            }
        }

        sstream << "그룹허가    ";
        break;
    }

    case options::ROAR:
        sstream << "외치기듣기  ";
        break;

    case options::ROAR_WORLDS:
        sstream << "세계후      ";
        break;

    case options::MAGIC_EFFECT:
        sstream << "마법이펙트  ";
        break;

    case options::WEATHER_EFFECT:
        sstream << "날씨변화    ";
        break;

    case options::FIXED_MOVE:
        sstream << "고정이동    ";
        break;

    case options::TRADE:
        sstream << "교환가능    ";
        break;

    case options::FAST_MOVE:
        sstream << "빠른이동    ";
        break;

    case options::EFFECT_SOUND:
        sstream << "소리듣기    ";
        break;

    case options::PK:
        sstream << "PK보호      ";
        break;

    default:
        return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    this->send(me, fb::protocol::game::response::message(sstream.str(), message::type::STATE), scope::SELF);
    this->send(me, fb::protocol::game::response::session::option(me), scope::SELF);
}

void fb::game::context::on_level_up(session& me)
{
    this->send(me, fb::protocol::game::response::object::effect(me, 0x02), scope::PIVOT);
}

void fb::game::context::on_warp(fb::game::session& me)
{
    auto map = me.map();
    if(map == nullptr)
        return;

    this->send(me, fb::protocol::game::response::session::id(me), scope::SELF);
    this->send(me, fb::protocol::game::response::map::config(*map), scope::SELF);
    this->send(me, fb::protocol::game::response::map::bgm(*map), scope::SELF);
    this->send(me, fb::protocol::game::response::session::position(me), scope::SELF);
    this->send(me, fb::protocol::game::response::session::show(me, me, false), scope::SELF);
    this->send(me, fb::protocol::game::response::object::direction(me), scope::SELF);
}

void fb::game::context::on_transfer(fb::game::session& me, fb::game::map& map, const point16_t& position)
{
    fb::ostream         parameter;
    parameter.write(me.name());

    auto& socket = static_cast<fb::socket<fb::game::session>&>(me);
    auto  fd     = socket.native_handle();
    this->_internal->send(fb::protocol::internal::request::transfer(me.name(), fb::protocol::internal::services::GAME, fb::protocol::internal::services::GAME, map.id(), position.x, position.y, fd));
}

void fb::game::context::on_item_get(session& me, const std::map<uint8_t, fb::game::item*>& items)
{ }

void fb::game::context::on_item_changed(session& me, const std::map<uint8_t, fb::game::item*>& items)
{ }

void fb::game::context::on_item_lost(session& me, const std::vector<uint8_t>& slots)
{ }

void fb::game::context::on_attack(mob& me, object* you)
{
    this->send(me, fb::protocol::game::response::life::action(me, action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT, true);
}

void fb::game::context::on_hit(mob& me, life& you, uint32_t damage, bool critical)
{
    you.hp_down(damage, &me, critical);
}

void fb::game::context::on_kill(mob& me, life& you)
{ }

void fb::game::context::on_damaged(mob& me, object* you, uint32_t damage, bool critical)
{ }

void fb::game::context::on_die(mob& me, object* you)
{
    this->send(me, fb::protocol::game::response::life::die(me), scope::PIVOT, true);
}

void fb::game::context::on_item_remove(session& me, uint8_t index, item::delete_attr attr)
{
    this->send(me, fb::protocol::game::response::item::remove(attr, index, 0), scope::SELF);
}

void fb::game::context::on_item_update(session& me, uint8_t index)
{
    this->send(me, fb::protocol::game::response::item::update(me, index), scope::SELF);
}

void fb::game::context::on_item_swap(session& me, uint8_t src, uint8_t dst)
{ }