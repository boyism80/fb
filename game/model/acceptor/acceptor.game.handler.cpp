#include "acceptor.game.h"

void fb::game::acceptor::on_create(fb::game::object& me)
{
    auto gd = std::lock_guard(this->_hash_mutex);

    if(this->_hash_set.contains(&me) == false)
        this->_hash_set.insert(&me);
}

void fb::game::acceptor::on_destroy(fb::game::object& me)
{
    auto gd = std::lock_guard(this->_hash_mutex);

    if(this->_hash_set.contains(&me))
        this->_hash_set.erase(&me);
}

void fb::game::acceptor::on_direction(fb::game::object& me)
{
    this->send(me, fb::protocol::game::response::object::direction(me), scope::PIVOT, true);
}

void fb::game::acceptor::on_show(fb::game::object& me, bool light)
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

void fb::game::acceptor::on_show(fb::game::object& me, fb::game::object& you, bool light)
{
    if(you.is(fb::game::object::types::SESSION))
        this->send(me, fb::protocol::game::response::session::show(static_cast<fb::game::session&>(you), me, light), scope::SELF);
    else
        this->send(me, fb::protocol::game::response::object::show(you), scope::SELF);
}

void fb::game::acceptor::on_hide(fb::game::object& me)
{
    this->send(me, fb::protocol::game::response::object::hide(me), scope::PIVOT);
}

void fb::game::acceptor::on_hide(fb::game::object& me, fb::game::object& you)
{
    this->send(me, fb::protocol::game::response::object::hide(you), scope::SELF);
}

void fb::game::acceptor::on_move(fb::game::object& me)
{
    this->send(me, fb::protocol::game::response::object::move(me), scope::PIVOT, true);
}

void fb::game::acceptor::on_unbuff(fb::game::object& me, fb::game::buff& buff)
{
     lua::get()
         .from("scripts/spell/%s.lua", buff.spell().uncast().c_str())
         .func("handle_uncast")
         .pushobject(me)
         .pushobject(buff.spell())
         .resume(2);
    this->send(me, fb::protocol::game::response::spell::unbuff(buff), scope::SELF);
}

void fb::game::acceptor::on_enter(fb::game::object& me, fb::game::map& map, const point16_t& position)
{
    auto thread = this->thread(map);
    
    if(thread == nullptr || thread == this->threads().current())
    {
        me.handle_enter(map, position);

        if(me.is(fb::game::object::types::SESSION))
            this->on_save(static_cast<fb::game::session&>(me));
    }
    else
    {
        thread->precedence.enqueue
        (
            [this, &me, &map, position] (uint8_t) 
            {
                me.handle_enter(map, position);
                if(me.is(fb::game::object::types::SESSION))
                    this->on_save(static_cast<fb::game::session&>(me));
            }
        );
    }
}

void fb::game::acceptor::on_attack(life& me, object* you)
{
}

void fb::game::acceptor::on_hit(life& me, life& you, uint32_t damage, bool critical)
{
}

void fb::game::acceptor::on_kill(life& me, life& you)
{
}

void fb::game::acceptor::on_damaged(life& me, object* you, uint32_t damage, bool critical)
{
    this->send(me, fb::protocol::game::response::life::show_hp(me, damage, false), scope::PIVOT);
}

void fb::game::acceptor::on_die(life& me, object* you)
{
}

void fb::game::acceptor::on_heal_hp(life& me, uint32_t value, fb::game::object* from)
{
}

void fb::game::acceptor::on_heal_mp(life& me, uint32_t value, fb::game::object* from)
{
}

void fb::game::acceptor::on_hp(life& me, uint32_t before, uint32_t current)
{
}

void fb::game::acceptor::on_mp(life& me, uint32_t before, uint32_t current)
{
}

void fb::game::acceptor::on_action(session& me, action action, duration duration, uint8_t sound)
{
    this->send(me, fb::protocol::game::response::session::action(me, action, duration), scope::PIVOT);
}

void fb::game::acceptor::on_updated(session& me, fb::game::state_level level)
{
    this->send(me, fb::protocol::game::response::session::state(me, level), scope::SELF);
}

void fb::game::acceptor::on_money_changed(session& me, uint32_t value)
{ }

void fb::game::acceptor::on_attack(session& me, object* you)
{
    this->send(me, fb::protocol::game::response::session::action(me, action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT);
    auto* weapon = me.items.weapon();
    if (weapon != nullptr)
    {
        auto            sound = weapon->sound();
        this->send(me, fb::protocol::game::response::object::sound(me, sound != 0 ? fb::game::sound::type(sound) : fb::game::sound::SWING), scope::PIVOT);
    }

    auto& thread = fb::game::lua::get();
    thread.from("scripts/common/attack.lua")
          .func("handle_attack")
          .pushobject(me);
    if(you != nullptr)
        thread.pushobject(*you);
    else
        thread.pushnil();
    thread.resume(2);
}

void fb::game::acceptor::on_hit(session& me, life& you, uint32_t damage, bool critical)
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

void fb::game::acceptor::on_kill(session& me, life& you)
{}

void fb::game::acceptor::on_damaged(session& me, object* you, uint32_t damage, bool critical)
{
}

void fb::game::acceptor::on_hold(session& me)
{
    this->send(me, fb::protocol::game::response::session::position(me), scope::SELF);
}

void fb::game::acceptor::on_die(session& me, object* you)
{
    me.state(state::GHOST);
}

void fb::game::acceptor::on_notify(session& me, const std::string& message, fb::game::message::type type)
{
    this->send(me, fb::protocol::game::response::message(message, type), scope::SELF);
}

void fb::game::acceptor::on_equipment_on(session& me, item& item, equipment::slot slot)
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

void fb::game::acceptor::on_equipment_off(session& me, equipment::slot slot, uint8_t index)
{
    this->send(me, fb::protocol::game::response::object::sound(me, sound::type::EQUIPMENT_OFF), scope::PIVOT);
}

void fb::game::acceptor::on_item_active(session& me, item& item)
{
    lua::get()
        .from("scripts/item/%s.lua", item.active_script().c_str())
        .func("handle_active")
        .pushobject(me)
        .pushobject(item)
        .resume(2);
}

void fb::game::acceptor::on_item_throws(session& me, item& item, const point16_t& to)
{
    if(me.position() != to)
        this->send(me, fb::protocol::game::response::session::throws(me, item, to), scope::PIVOT);
    else
        this->send(me, fb::protocol::game::response::session::action(me, fb::game::action::ATTACK, fb::game::duration::DURATION_THROW), scope::PIVOT);
}

void fb::game::acceptor::on_spell_update(life& me, uint8_t index)
{
    this->send(me, fb::protocol::game::response::spell::update(me, index), scope::SELF);
}

void fb::game::acceptor::on_spell_remove(life& me, uint8_t index)
{
    this->send(me, fb::protocol::game::response::spell::remove(me, index), scope::SELF);
}

void fb::game::acceptor::on_trade_begin(session& me, session& you)
{
    this->send(me, fb::protocol::game::response::trade::dialog(you), scope::SELF);
}

void fb::game::acceptor::on_trade_bundle(session& me)
{
    this->send(me, fb::protocol::game::response::trade::bundle(), scope::SELF);
}

void fb::game::acceptor::on_trade_money(session& me, session& from)
{
    bool mine = (&me == &from);
    this->send(me, fb::protocol::game::response::trade::money(from, mine), scope::SELF);
}

void fb::game::acceptor::on_trade_cancel(session& me, session& from)
{
    bool mine = (&me == &from);
    this->send(me, fb::protocol::game::response::trade::close(mine ? fb::game::message::trade::CANCELLED_BY_ME : fb::game::message::trade::CANCELLED_BY_PARTNER), scope::SELF);
}

void fb::game::acceptor::on_trade_lock(session& me, bool mine)
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

void fb::game::acceptor::on_trade_failed(session& me)
{
    this->send(me, fb::protocol::game::response::trade::close(fb::game::message::trade::FAILED), scope::SELF);
}

void fb::game::acceptor::on_trade_success(session& me)
{
    this->send(me, fb::protocol::game::response::trade::close(fb::game::message::trade::SUCCESS), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::common(object, message, button_prev, button_next, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::menu(npc, menus, message, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::slot(npc, item_slots, message, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::item(npc, cores, message, 0xFFFF, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message,  fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::input(npc, message, interaction), scope::SELF);
}

void fb::game::acceptor::on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::input_ext(npc, message, top, bottom, maxlen, prev, interaction), scope::SELF);
}

void fb::game::acceptor::on_trade_item(session& me, session& from, uint8_t index)
{
    bool mine = (&me == &from);
    this->send(me, fb::protocol::game::response::trade::upload(from, index, mine), scope::SELF);
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

void fb::game::acceptor::on_level_up(session& me)
{
    this->send(me, fb::protocol::game::response::object::effect(me, 0x02), scope::PIVOT);
}

void fb::game::acceptor::on_warp(fb::game::session& me)
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

void fb::game::acceptor::on_transfer(fb::game::session& me, fb::game::map& map, const point16_t& position)
{
    fb::ostream         parameter;
    parameter.write(me.name());

    auto& socket = static_cast<fb::socket<fb::game::session>&>(me);
    auto fd = socket.native_handle();
    this->_internal->send(fb::protocol::internal::request::transfer(me.name(), fb::protocol::internal::services::SERVICE_GAME, fb::protocol::internal::services::SERVICE_GAME, map.id(), position.x, position.y, fd));
}

void fb::game::acceptor::on_item_get(session& me, const std::map<uint8_t, fb::game::item*>& items)
{
}

void fb::game::acceptor::on_item_changed(session& me, const std::map<uint8_t, fb::game::item*>& items)
{ }

void fb::game::acceptor::on_item_lost(session& me, const std::vector<uint8_t>& slots)
{ }

void fb::game::acceptor::on_attack(mob& me, object* you)
{
    this->send(me, fb::protocol::game::response::life::action(me, action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT, true);
}

void fb::game::acceptor::on_hit(mob& me, life& you, uint32_t damage, bool critical)
{
    you.hp_down(damage, &me, critical);
}

void fb::game::acceptor::on_kill(mob& me, life& you)
{}

void fb::game::acceptor::on_damaged(mob& me, object* you, uint32_t damage, bool critical)
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
        auto                    range = fb::game::table::classes.exp(slayer.cls(), slayer.level());
        // 3.3% 제한한 경험치
        auto                    exp = me.experience();
#if defined DEBUG | defined _DEBUG
        exp *= 100;
#else
        if (slayer.max_level() == false)
            exp = std::min(uint32_t(range / 100.0f * 3.3f + 1), exp);
#endif
        slayer.experience_add(exp, true);
    }
}

void fb::game::acceptor::on_die(mob& me, object* you)
{
    // 몹 체력을 다 깎았으면 죽인다.
    this->send(me, fb::protocol::game::response::life::die(me), scope::PIVOT, true);
    me.dead_time(std::chrono::duration_cast<std::chrono::milliseconds>(fb::thread::now()));

    // 드롭 아이템 떨구기
    std::vector<object*> dropped_items;
    for(auto candidate : me.items())
    {
        if(std::rand() % 100 > candidate.percentage)
            continue;

        auto            item = static_cast<fb::game::item*>(candidate.item->make(this));
        item->map(me.map(), me.position());

        dropped_items.push_back(item);
    }

    if(dropped_items.size() != 0)
        this->send(me, fb::protocol::game::response::object::show(dropped_items), scope::PIVOT, true);
}

void fb::game::acceptor::on_item_remove(session& me, uint8_t index, item::delete_attr attr)
{
    this->send(me, fb::protocol::game::response::item::remove(attr, index, 0), scope::SELF);
}

void fb::game::acceptor::on_item_update(session& me, uint8_t index)
{
    this->send(me, fb::protocol::game::response::item::update(me, index), scope::SELF);
}

void fb::game::acceptor::on_item_swap(session& me, uint8_t src, uint8_t dst)
{ }

void fb::game::acceptor::on_save(session& me)
{
    db::query(me.name().c_str(), std::vector<std::string>
    {
        service::sql::session::update(me),
        service::sql::item::update(me),
        service::sql::spell::update(me),
    });
}