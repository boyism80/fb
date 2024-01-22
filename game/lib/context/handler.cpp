#include <fb/game/context.h>

void fb::game::context::on_create(fb::game::object& me)
{
    auto gd = std::lock_guard(this->_hash_mutex);

    if(this->_objects.contains(&me) == false)
        this->_objects.insert(std::make_pair(&me, std::unique_ptr<fb::game::object>(&me)));
}

void fb::game::context::on_destroy(fb::game::object& me)
{
    
}

void fb::game::context::on_chat(fb::game::object& me, const std::string& message, bool shout)
{
    auto sstream = std::stringstream();
    if (shout)
    {
        sstream << me.name() << "! " << message;
    }
    else
    {
        sstream << me.name() << ": " << message;
    }

    this->send(me, fb::protocol::game::response::chat(me, sstream.str(), shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL), shout ? scope::MAP : scope::PIVOT);
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
            [&me, light](const auto& to)
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

void fb::game::context::on_move(fb::game::object& me, const point16_t& before)
{
    this->send(me, fb::protocol::game::response::object::move(me, before), scope::PIVOT, true);
}

void fb::game::context::on_unbuff(fb::game::object& me, fb::game::buff& buff)
{
    if(buff.spell.uncast.empty())
        return;

     auto thread = lua::get();
     if(thread == nullptr)
         return;
     thread->from(buff.spell.uncast.c_str())
         .func("on_uncast")
         .pushobject(me)
         .pushobject(buff.spell)
         .resume(2);
    this->send(me, fb::protocol::game::response::spell::unbuff(buff), scope::SELF);
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

void fb::game::context::on_action(session& me, ACTION_TYPE action, DURATION duration, uint8_t sound)
{
    this->send(me, fb::protocol::game::response::session::action(me, action, duration), scope::PIVOT);
}

void fb::game::context::on_updated(session& me, fb::game::STATE_LEVEL level)
{
    this->send(me, fb::protocol::game::response::session::state(me, level), scope::SELF);
}

void fb::game::context::on_money_changed(session& me, uint32_t value)
{ }

void fb::game::context::on_attack(session& me, object* you)
{
    this->send(me, fb::protocol::game::response::session::action(me, ACTION_TYPE::ATTACK, DURATION::ATTACK), scope::PIVOT);
    auto* weapon = me.items.weapon();
    if (weapon != nullptr)
    {
        auto            sound = weapon->based<fb::game::weapon>()->sound;
        this->send(me, fb::protocol::game::response::object::sound(me, sound != 0 ? fb::game::SOUND_TYPE(sound) : fb::game::SOUND_TYPE
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
        this->send(me, fb::protocol::game::response::object::sound(me, SOUND_TYPE::DAMAGE), scope::PIVOT);

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

void fb::game::context::on_notify(session& me, const std::string& message, fb::game::MESSAGE_TYPE type)
{
    this->send(me, fb::protocol::game::response::message(message, type), scope::SELF);
}

void fb::game::context::on_equipment_on(session& me, item& item, equipment::slot slot)
{
    this->send(me, fb::protocol::game::response::item::update_slot(me, slot), scope::SELF);
    this->send(me, fb::protocol::game::response::object::sound(me, SOUND_TYPE::EQUIPMENT_ON), scope::PIVOT);

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
    this->send(me, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);

    sstream.str(std::string());
    sstream << "갑옷 강도  " << me.defensive_physical() <<"  " << me.regenerative() << " S  " << me.defensive_magical();
    this->send(me, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
}

void fb::game::context::on_equipment_off(session& me, equipment::slot slot, uint8_t index)
{
    this->send(me, fb::protocol::game::response::object::sound(me, SOUND_TYPE::EQUIPMENT_OFF), scope::PIVOT);
}

void fb::game::context::on_item_active(session& me, item& item)
{
    auto thread = lua::get();
    if(thread == nullptr)
        return;

    thread->from(item.based<fb::game::item>()->active_script.c_str())
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
        this->send(me, fb::protocol::game::response::session::action(me, fb::game::ACTION_TYPE::ATTACK, fb::game::DURATION::THROW), scope::PIVOT);
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
        this->send(me, fb::protocol::game::response::message(fb::game::message::trade::NOTIFY_LOCK_TO_PARTNER, MESSAGE_TYPE::POPUP), scope::SELF);
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

void fb::game::context::on_dialog(session& me, const object::model& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::common(object, message, button_prev, button_next, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::menu(npc, menus, message, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::slot(npc, item_slots, message, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const fb::game::dialog::item_pairs& pairs, uint16_t pursuit, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::item(npc, pairs, message, pursuit, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message,  fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::input(npc, message, interaction), scope::SELF);
}

void fb::game::context::on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    this->send(me, fb::protocol::game::response::dialog::input_ext(npc, message, top, bottom, maxlen, prev, interaction), scope::SELF);
}

void fb::game::context::on_trade_item(session& me, session& from, uint8_t index)
{
    bool mine = (&me == &from);
    this->send(me, fb::protocol::game::response::trade::upload(from, index, mine), scope::SELF);
}

void fb::game::context::on_option(session& me, fb::game::OPTION option, bool enabled)
{
    std::stringstream sstream;

    switch(option)
    {
    case OPTION::WHISPER:
        sstream << "귓속말듣기  ";
        break;

    case OPTION::GROUP:
    {
        auto group = me.group();
        if(group != nullptr)
        {
            if(group->members().size() == 1)
            {
                this->send(me, fb::protocol::game::response::message("그룹 해체", MESSAGE_TYPE::STATE), scope::GROUP);
                fb::game::group::destroy(*group);
            }
            else
            {
                auto leader = group->leave(me);
                std::stringstream sstream;
                sstream << me.name() << "님 그룹 탈퇴";
                this->send(*leader, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
            }
        }

        sstream << "그룹허가    ";
        break;
    }

    case OPTION::ROAR:
        sstream << "외치기듣기  ";
        break;

    case OPTION::ROAR_WORLDS:
        sstream << "세계후      ";
        break;

    case OPTION::MAGIC_EFFECT:
        sstream << "마법이펙트  ";
        break;

    case OPTION::WEATHER_EFFECT:
        sstream << "날씨변화    ";
        break;

    case OPTION::FIXED_MOVE:
        sstream << "고정이동    ";
        break;

    case OPTION::TRADE:
        sstream << "교환가능    ";
        break;

    case OPTION::FAST_MOVE:
        sstream << "빠른이동    ";
        break;

    case OPTION::EFFECT_SOUND:
        sstream << "소리듣기    ";
        break;

    case OPTION::PK:
        sstream << "PK보호      ";
        break;

    default:
        return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    this->send(me, fb::protocol::game::response::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
    this->send(me, fb::protocol::game::response::session::option(me), scope::SELF);
}

void fb::game::context::on_level_up(session& me)
{
    this->send(me, fb::protocol::game::response::object::effect(me, 0x02), scope::PIVOT);
}

void fb::game::context::on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after)
{
    if(after == nullptr)
        return;

    if(me.is(fb::game::object::types::SESSION) == false)
        return;

    auto& session = static_cast<fb::game::session&>(me);
    this->send(session, fb::protocol::game::response::session::id(session), scope::SELF);
    this->send(session, fb::protocol::game::response::map::config(*after), scope::SELF);
    this->send(session, fb::protocol::game::response::map::bgm(*after), scope::SELF);
    this->send(session, fb::protocol::game::response::session::position(session), scope::SELF);
    this->send(session, fb::protocol::game::response::session::show(session, session, false), scope::SELF);
    this->send(session, fb::protocol::game::response::object::direction(session), scope::SELF);

    if(before == nullptr)
        this->save(session);
}

fb::task<void> fb::game::context::co_transfer(fb::game::session& me, fb::game::map& map, const point16_t& position, fb::awaitable<bool>* awaitable)
{
    fb::ostream         parameter;
    parameter.write(me.name());

    auto& socket   = static_cast<fb::socket<fb::game::session>&>(me);
    auto  fd       = socket.native_handle();
    auto  request  = fb::protocol::internal::request::transfer(me.name(), fb::protocol::internal::services::GAME, fb::protocol::internal::services::GAME, map.id(), position.x, position.y, fd);
    
    try
    {
        auto result = false;
        auto response = co_await this->request<fb::protocol::internal::response::transfer>(request, true, true);
        auto socket = this->sockets[response.fd];
        if(socket == nullptr)
        {
            if(awaitable != nullptr)
            {
                awaitable->result = &result;
                awaitable->handler.resume();
            }
            co_return;
        }

        if(response.code != fb::protocol::internal::response::transfer_code::SUCCESS)
            throw std::runtime_error("비바람이 휘몰아치고 있습니다.");

        auto session = socket->data();
        co_await session->co_map(nullptr);
        
        co_await this->co_save(*session);
        fb::ostream         parameter;
        parameter.write_u32(me.id());
        parameter.write(response.name);
        parameter.write_u8(1);
        parameter.write_u16(response.map);
        parameter.write_u16(response.x);
        parameter.write_u16(response.y);
        this->transfer(*socket, response.ip, response.port, fb::protocol::internal::services::GAME, parameter);
        
        result = true;
        if(awaitable != nullptr)
        {
            awaitable->result = &result;
            awaitable->handler.resume();
        }
    }
    catch(std::exception& e)
    {
        auto result = false;
        auto client = this->sockets[fd];
        if(client == nullptr)
        {
            if(awaitable != nullptr)
            {
                awaitable->result = &result;
                awaitable->handler.resume();
            }
            co_return;
        }

        auto message = e.what();
        auto session = client->data();
        session->refresh_map();
        this->on_notify(*session, message, fb::game::MESSAGE_TYPE::STATE);

        if(awaitable != nullptr)
        {
            awaitable->result = &result;
            awaitable->handler.resume();
        }
    }
    catch(boost::system::error_code& e)
    {
        auto result = false;
        auto client = this->sockets[fd];
        if(client == nullptr)
        {
            if(awaitable != nullptr)
            {
                awaitable->result = &result;
                awaitable->handler.resume();
            }
            co_return;
        }

        auto message = "비바람이 휘몰아치고 있습니다.";
        auto session = client->data();
        session->refresh_map();
        this->on_notify(*session, message, fb::game::MESSAGE_TYPE::STATE);

        if(awaitable != nullptr)
        {
            awaitable->result = &result;
            awaitable->handler.resume();
        }
    }
}

void fb::game::context::on_transfer(fb::game::session& me, fb::game::map& map, const point16_t& position, fb::awaitable<bool>* awaitable)
{
    this->co_transfer(me, map, position, awaitable);
}

void fb::game::context::on_item_get(session& me, const std::map<uint8_t, fb::game::item*>& items)
{ }

void fb::game::context::on_item_changed(session& me, const std::map<uint8_t, fb::game::item*>& items)
{ }

void fb::game::context::on_item_lost(session& me, const std::vector<uint8_t>& slots)
{ }

void fb::game::context::on_attack(mob& me, object* you)
{
    this->send(me, fb::protocol::game::response::life::action(me, ACTION_TYPE::ATTACK, DURATION::ATTACK), scope::PIVOT, true);
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

void fb::game::context::on_item_remove(session& me, uint8_t index, fb::game::item::DELETE_TYPE attr)
{
    this->send(me, fb::protocol::game::response::item::remove(attr, index, 0), scope::SELF);
}

void fb::game::context::on_item_update(session& me, uint8_t index)
{
    this->send(me, fb::protocol::game::response::item::update(me, index), scope::SELF);
}

void fb::game::context::on_item_swap(session& me, uint8_t src, uint8_t dst)
{ }