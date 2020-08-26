#include "acceptor.game.h"

void fb::game::acceptor::on_direction(fb::game::object& me)
{
    this->send(me, response::game::object::direction(me), scope::PIVOT, true);
}

void fb::game::acceptor::on_show(fb::game::object& me, bool light)
{
    if(me.is(fb::game::object::types::SESSION))
        this->send(me, response::game::session::show(static_cast<fb::game::session&>(me), light), scope::PIVOT);
    else
        this->send(me, response::game::object::show(me), scope::PIVOT);
}

void fb::game::acceptor::on_show(fb::game::object& me, fb::game::object& you, bool light)
{
    if(you.is(fb::game::object::types::SESSION))
        this->send(me, response::game::session::show(static_cast<fb::game::session&>(you), light), scope::SELF);
    else
        this->send(me, response::game::object::show(you), scope::SELF);
}

void fb::game::acceptor::on_hide(fb::game::object& me)
{
    this->send(me, response::game::object::hide(me), scope::PIVOT);
}

void fb::game::acceptor::on_hide(fb::game::object& me, fb::game::object& you)
{
    this->send(me, response::game::object::hide(you), scope::SELF);
}

void fb::game::acceptor::on_move(fb::game::object& me)
{
    this->send(me, response::game::object::move(me), scope::PIVOT, true);
}

void fb::game::acceptor::on_warp(fb::game::object& me)
{
    auto map = me.map();

    if(me.is(object::types::SESSION))
    {
        auto& session = static_cast<game::session&>(me);
        this->send(session, response::game::session::id(session), scope::SELF);
        this->send(session, response::game::map::config(*map), scope::SELF);
        this->send(session, response::game::map::bgm(*map), scope::SELF);
        this->send(session, response::game::session::state(session, state_level::LEVEL_MAX), scope::SELF);
        this->send(session, response::game::session::position(session), scope::SELF);
        this->send(session, response::game::session::show(session, false), scope::SELF);
        this->send(session, response::game::object::direction(session), scope::SELF);
    }
}

void fb::game::acceptor::on_unbuff(fb::game::object& me, fb::game::buff& buff)
{
     lua::thread()
         .from("scripts/spell/%s.lua", buff.spell().uncast().c_str())
         .func("handle_uncast")
         .pushobject(me)
         .pushobject(buff.spell())
         .resume(2);
    this->send(me, response::game::spell::unbuff(buff), scope::SELF);
}

void fb::game::acceptor::on_attack(life& me, object* you, uint32_t damage, bool critical)
{
}

void fb::game::acceptor::on_damage(life& me, object* you, uint32_t damage, bool critical)
{
    this->send(me, response::game::life::show_hp(me, damage, false), scope::PIVOT);
}

void fb::game::acceptor::on_heal_hp(life& me, uint32_t value, fb::game::object* from)
{
}

void fb::game::acceptor::on_heal_mp(life& me, uint32_t value, fb::game::object* from)
{
}

void fb::game::acceptor::on_die(life& me)
{}

void fb::game::acceptor::on_hp(life& me, uint32_t before, uint32_t current)
{
}

void fb::game::acceptor::on_mp(life& me, uint32_t before, uint32_t current)
{
}

void fb::game::acceptor::on_action(session& me, action action, duration duration, uint8_t sound)
{
    this->send(me, response::game::session::action(me, action, duration), scope::PIVOT);
}

void fb::game::acceptor::on_updated(session& me, fb::game::state_level level)
{
    this->send(me, response::game::session::state(me, level), scope::SELF);
}

void fb::game::acceptor::on_attack(session& me, object* you, uint32_t damage, bool critical)
{
    this->send(me, response::game::session::action(me, action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT);
    auto* weapon = me.items.weapon();
    if (weapon != nullptr)
    {
        auto            sound = weapon->sound();
        this->send(me, response::game::object::sound(me, sound != 0 ? game::sound::type(sound) : game::sound::SWING), scope::PIVOT);
    }

#if !defined DEBUG && !defined _DEBUG

#endif

    try
    {
        if(you == nullptr)
            throw std::exception();

        if(you->is(object::types::LIFE) == false)
            throw std::exception();

        auto life = static_cast<fb::game::life*>(you);
        if(life->alive() == false)
            throw std::exception();

#ifndef DEBUG
        auto miss = (std::rand() % 3 == 0);
        if(miss)
            throw std::exception();
#endif

#ifndef PK
        if(life->is(object::types::SESSION))
            throw std::exception();
#endif


        if (weapon != nullptr)
            this->send(me, response::game::object::sound(me, sound::type::DAMAGE), scope::PIVOT);

        life->hp_down(damage, &me, critical);
    }
    catch(std::exception&)
    {}

    lua::thread         thread;
    thread.from("scripts/common/attack.lua")
          .func("handle_attack")
          .pushobject(me);
    if(you != nullptr)
        thread.pushobject(*you);
    else
        thread.pushnil();
    thread.resume(2);
}

void fb::game::acceptor::on_damage(session& me, object* you, uint32_t damage, bool critical)
{
}

void fb::game::acceptor::on_hold(session& me)
{
    this->send(me, response::game::session::position(me), scope::SELF);
}

void fb::game::acceptor::on_die(session& me)
{
    me.state(state::GHOST);
    this->send(me, response::game::session::show(me, true), scope::SELF);
}

void fb::game::acceptor::on_notify(session& me, const std::string& message, game::message::type type)
{
    this->send(me, response::game::message(message, type), scope::SELF);
}

void fb::game::acceptor::on_equipment_on(session& me, item& item, equipment::slot slot)
{
    this->send(me, response::game::item::update_slot(me, slot), scope::SELF);
    this->send(me, response::game::object::sound(me, sound::type::EQUIPMENT_ON), scope::PIVOT);

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
    this->send(me, response::game::message(sstream.str(), message::type::STATE), scope::SELF);

    sstream.str(std::string());
    sstream << "갑옷 강도  " << me.defensive_physical() <<"  " << me.regenerative() << " S  " << me.defensive_magical();
    this->send(me, response::game::message(sstream.str(), message::type::STATE), scope::SELF);
}

void fb::game::acceptor::on_equipment_off(session& me, equipment::slot slot)
{
    this->send(me, response::game::object::sound(me, sound::type::EQUIPMENT_OFF), scope::PIVOT);
}

void fb::game::acceptor::on_item_active(session& me, item& item)
{
    lua::thread()
        .from("scripts/item/%s.lua", item.active_script().c_str())
        .func("handle_active")
        .pushobject(me)
        .pushobject(item)
        .resume(2);
}

void fb::game::acceptor::on_item_throws(session& me, item& item, const point16_t& to)
{
    this->send(me, response::game::session::throws(me, item, to), scope::PIVOT);
}

void fb::game::acceptor::on_spell_update(life& me, uint8_t index)
{
    this->send(me, response::game::spell::update(me, index), scope::SELF);
}

void fb::game::acceptor::on_spell_remove(life& me, uint8_t index)
{
    this->send(me, response::game::spell::remove(me, index), scope::SELF);
}

void fb::game::acceptor::on_trade_begin(session& me, session& you)
{
    this->send(me, response::game::trade::dialog(you), scope::SELF);
}

void fb::game::acceptor::on_trade_bundle(session& me)
{
    this->send(me, response::game::trade::bundle(), scope::SELF);
}

void fb::game::acceptor::on_trade_money(session& me, session& from)
{
    bool mine = (&me == &from);
    this->send(me, response::game::trade::money(from, mine), scope::SELF);
}

void fb::game::acceptor::on_trade_cancel(session& me)
{
    this->send(me, response::game::trade::close(message::trade::CANCELLED_BY_ME), scope::SELF);
}

void fb::game::acceptor::on_trade_lock(session& me, bool mine)
{
    if(mine)
    {
        this->send(me, response::game::trade::lock(), scope::SELF);
    }
    else
    {
        this->send(me, response::game::message(message::trade::NOTIFY_LOCK_TO_PARTNER, message::type::POPUP), scope::SELF);
    }
}

void fb::game::acceptor::on_trade_failed(session& me)
{
    this->send(me, response::game::trade::close(message::trade::FAILED), scope::SELF);
}

void fb::game::acceptor::on_trade_success(session& me)
{
    this->send(me, response::game::trade::close(message::trade::SUCCESS), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    this->send(me, response::game::dialog::common(object, message, button_prev, button_next, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    this->send(me, response::game::dialog::menu(npc, menus, message, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    this->send(me, response::game::dialog::slot(npc, item_slots, message, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction)
{
    this->send(me, response::game::dialog::item(npc, cores, message, 0xFFFF, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message,  fb::game::dialog::interaction interaction)
{
    this->send(me, response::game::dialog::input(npc, message, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    this->send(me, response::game::dialog::input_ext(npc, message, top, bottom, maxlen, prev, interaction), scope::SELF);
}

void fb::game::acceptor::on_trade_item(session& me, session& from, uint8_t index)
{
    bool mine = (&me == &from);
    this->send(me, response::game::trade::upload(from, index, mine), scope::SELF);
}

void fb::game::acceptor::on_option(session& me, fb::game::options option, bool enabled)
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
                this->send(me, response::game::message("그룹 해체", message::type::STATE), scope::GROUP);
                fb::game::group::destroy(*group);
            }
            else
            {
                auto leader = group->leave(me);
                std::stringstream sstream;
                sstream << me.name() << "님 그룹 탈퇴";
                this->send(*leader, response::game::message(sstream.str(), message::type::STATE), scope::GROUP);
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
        sstream << "고정이동     ";
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
    this->send(me, response::game::message(sstream.str(), message::type::STATE), scope::SELF);
    this->send(me, response::game::session::option(me), scope::SELF);
}

void fb::game::acceptor::on_level_up(session& me)
{
    this->send(me, response::game::object::effect(me, 0x02), scope::PIVOT);
}

void fb::game::acceptor::on_attack(mob& me, object* you, uint32_t damage, bool critical)
{
}

void fb::game::acceptor::on_damage(mob& me, object* you, uint32_t damage, bool critical)
{
    if(me.alive())
    {
        if(me.offensive() != fb::game::mob::offensive_type::NONE && you != nullptr)
        {
            me.target(static_cast<fb::game::life*>(you));
        }
    }
    else if(you != nullptr && you->is(object::types::SESSION))
    {
        auto&                   slayer = static_cast<fb::game::session&>(*you);
        // 3.3% 제한한 경험치
        auto                    exp = me.experience();
#if defined DEBUG | defined _DEBUG
        exp *= 100;
#else
        if (me.max_level() == false)
            exp = std::min(uint32_t(exp_range / 100.0f * 3.3f + 1), exp);
#endif
        slayer.experience_add(exp, true);
    }
}

void fb::game::acceptor::on_die(mob& me)
{
    // 몹 체력을 다 깎았으면 죽인다.
    this->send(me, response::game::life::die(me), scope::PIVOT, true);
    me.dead_time(fb::timer::now());

    // 드롭 아이템 떨구기
    std::vector<object*> dropped_items;
    for(auto candidate : me.items())
    {
        if(std::rand() % 100 > candidate.percentage)
            continue;

        auto            item = static_cast<fb::game::item*>(candidate.item->make(this));
        me.map()->objects.add(*item, me.position());

        dropped_items.push_back(item);
    }

    if(dropped_items.size() != 0)
        this->send(me, response::game::object::show(dropped_items), scope::PIVOT, true);
}

void fb::game::acceptor::on_item_remove(session& me, uint8_t index, item::delete_attr attr)
{
    this->send(me, response::game::item::remove(attr, index, 0), scope::SELF);
}

void fb::game::acceptor::on_item_update(session& me, uint8_t index)
{
    this->send(me, response::game::item::update(me, index), scope::SELF);
}
