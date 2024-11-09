#include <context.h>

using namespace fb::game;

void context::on_create(object& me)
{ }

void context::on_destroy(object& me)
{ }

void context::on_chat(object& me, const std::string& message, bool shout)
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

    this->send(me,
               fb_resp::chat(me, sstream.str(), shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL),
               shout ? scope::MAP : scope::PIVOT);
}

void context::on_direction(object& me)
{
    this->send(me, fb_resp::object::direction(me), scope::PIVOT, true);
}

void context::on_show(object& me, bool light)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        this->send(
            me,
            [&me, light](const auto& to) {
                return std::unique_ptr<fb::protocol::base::header>(
                    new fb_resp::session::show(static_cast<character&>(me), to, light));
            },
            scope::PIVOT);
    }
    else
    {
        this->send(me, fb_resp::object::show(me), scope::PIVOT);
    }
}

void context::on_show(object& me, object& you, bool light)
{
    if (you.is(OBJECT_TYPE::CHARACTER))
        this->send(me, fb_resp::session::show(static_cast<character&>(you), me, light), scope::SELF);
    else
        this->send(me, fb_resp::object::show(you), scope::SELF);
}

void context::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        this->send(me, fb_resp::object::hide(me), scope::PIVOT, true);
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->send(me, fb_resp::life::die(static_cast<life&>(me)), scope::PIVOT, true);
        break;
    }
}

void context::on_hide(object& me, object& you, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        this->send(me, fb_resp::object::hide(you), scope::SELF);
        break;

    case DESTROY_TYPE::DEAD:
        if (you.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->send(me, fb_resp::life::die(static_cast<life&>(you)), scope::SELF);
        break;
    }
}

void context::on_move(object& me, const point16_t& before)
{
    this->send(me, fb_resp::object::move(me, before), scope::PIVOT, true);
}

void context::on_unbuff(object& me, buff& buff)
{
    if (buff.model.uncast.empty())
        return;

    auto thread = lua::get();
    if (thread == nullptr)
        return;
    thread->from(buff.model.uncast.c_str()).func("on_uncast").pushobject(me).pushobject(buff.model).resume(2);
    this->send(me, fb_resp::spell::unbuff(buff), scope::SELF);
}

void context::on_attack(life& me, object* you)
{
    switch (me.what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        this->send(me,
                   fb_resp::session::action(static_cast<character&>(me), ACTION::ATTACK, DURATION::ATTACK),
                   scope::PIVOT);
        auto* weapon = static_cast<character&>(me).items.weapon();
        if (weapon != nullptr)
        {
            auto sound = weapon->based<fb::model::weapon>().sound;
            this->send(me, fb_resp::object::sound(me, sound != 0 ? SOUND(sound) : SOUND::SWING), scope::PIVOT);
        }
    }
    break;

    case OBJECT_TYPE::MOB:
        this->send(me, fb_resp::life::action(me, ACTION::ATTACK, DURATION::ATTACK), scope::PIVOT, true);
        break;
    }
}

void context::on_hit(life& me, life& you, uint32_t damage, bool critical)
{
    switch (me.what())
    {
    case OBJECT_TYPE::MOB:
    {
        you.hp_down(damage, &me, critical);
    }

    case OBJECT_TYPE::CHARACTER:
    {
#ifndef PK
        if (you.is(OBJECT_TYPE::CHARACTER))
            return;
#endif

        auto* weapon = static_cast<character&>(me).items.weapon();
        if (weapon != nullptr)
            this->send(me, fb_resp::object::sound(me, SOUND::DAMAGE), scope::PIVOT);

        you.hp_down(damage, &me, critical);
    }
    break;
    }
}

void context::on_kill(life& me, life& you)
{ }

void context::on_damaged(life& me, object* you, uint32_t damage, bool critical)
{
    this->send(me, fb_resp::life::show_hp(me, damage, false), scope::PIVOT);
}

void context::on_die(life& me, object* you)
{ }

void context::on_heal_hp(life& me, uint32_t value, object* from)
{ }

void context::on_heal_mp(life& me, uint32_t value, object* from)
{ }

void context::on_hp(life& me, uint32_t before, uint32_t current)
{ }

void context::on_mp(life& me, uint32_t before, uint32_t current)
{ }

void context::on_action(character& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->send(me, fb_resp::session::action(me, action, duration), scope::PIVOT);
}

void context::on_updated(character& me, STATE_LEVEL level)
{
    this->send(me, fb_resp::session::state(me, level), scope::SELF);
}

void context::on_money_changed(character& me, uint32_t value)
{ }

void context::on_hold(character& me)
{
    this->send(me, fb_resp::session::position(me), scope::SELF);
}

void context::on_notify(character& me, const std::string& message, MESSAGE_TYPE type)
{
    this->send(me, fb_resp::message(message, type), scope::SELF);
}

void context::on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)
{
    this->send(me, fb_resp::item::update_slot(me, parts), scope::SELF);
    this->send(me, fb_resp::object::sound(me, SOUND::EQUIPMENT_ON), scope::PIVOT);

    std::stringstream sstream;
    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        sstream << "w:무기  :";
        break;

    case EQUIPMENT_PARTS::ARMOR:
        sstream << "a:갑옷  :";
        break;

    case EQUIPMENT_PARTS::SHIELD:
        sstream << "s:방패  :";
        break;

    case EQUIPMENT_PARTS::HELMET:
        sstream << "h:머리  :";
        break;

    case EQUIPMENT_PARTS::LEFT_HAND:
        sstream << "l:왼손  :";
        break;

    case EQUIPMENT_PARTS::RIGHT_HAND:
        sstream << "r:오른손  :";
        break;

    case EQUIPMENT_PARTS::LEFT_AUX:
        sstream << "[:보조1  :";
        break;

    case EQUIPMENT_PARTS::RIGHT_AUX:
        sstream << "]:보조2  :";
        break;
    }

    sstream << item.name();
    this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);

    sstream.str(std::string());
    sstream << "갑옷 강도  " << me.defensive_physical() << "  " << me.regenerative() << " S  "
            << me.defensive_magical();
    this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
}

void context::on_equipment_off(character& me, EQUIPMENT_PARTS parts, uint8_t index)
{
    this->send(me, fb_resp::object::sound(me, SOUND::EQUIPMENT_OFF), scope::PIVOT);
}

void context::on_item_active(character& me, item& item)
{
    auto thread = lua::get();
    if (thread == nullptr)
        return;

    thread->from(item.based<fb::model::item>().script_active.c_str())
        .func("on_active")
        .pushobject(me)
        .pushobject(item)
        .resume(2);
}

void context::on_item_throws(character& me, item& item, const point16_t& to)
{
    if (me.position() != to)
        this->send(me, fb_resp::session::throws(me, item, to), scope::PIVOT);
    else
        this->send(me, fb_resp::session::action(me, ACTION::ATTACK, DURATION::THROW), scope::PIVOT);
}

void context::on_spell_update(life& me, uint8_t index)
{
    this->send(me, fb_resp::spell::update(me, index), scope::SELF);
}

void context::on_spell_remove(life& me, uint8_t index)
{
    this->send(me, fb_resp::spell::remove(me, index), scope::SELF);
}

void context::on_trade_begin(character& me, character& you)
{
    this->send(me, fb_resp::trade::dialog(you, this->model), scope::SELF);
}

void context::on_trade_bundle(character& me)
{
    this->send(me, fb_resp::trade::bundle(), scope::SELF);
}

void context::on_trade_money(character& me, character& from)
{
    bool mine = (&me == &from);
    this->send(me, fb_resp::trade::money(from, mine), scope::SELF);
}

void context::on_trade_cancel(character& me, character& from)
{
    bool mine = (&me == &from);
    this->send(me,
               fb_resp::trade::close(mine ? message::trade::CANCELLED_BY_ME : message::trade::CANCELLED_BY_PARTNER),
               scope::SELF);
}

void context::on_trade_lock(character& me, bool mine)
{
    if (mine)
    {
        this->send(me, fb_resp::trade::lock(), scope::SELF);
    }
    else
    {
        this->send(me, fb_resp::message(message::trade::NOTIFY_LOCK_TO_PARTNER, MESSAGE_TYPE::POPUP), scope::SELF);
    }
}

void context::on_trade_failed(character& me)
{
    this->send(me, fb_resp::trade::close(message::trade::FAILED), scope::SELF);
}

void context::on_trade_success(character& me)
{
    this->send(me, fb_resp::trade::close(message::trade::SUCCESS), scope::SELF);
}

// new dialog
void context::on_dialog(character&               me,
                        const fb::model::object& object,
                        const std::string&       message,
                        bool                     button_prev,
                        bool                     button_next,
                        dialog::interaction      interaction)
{
    this->send(me, fb_resp::dialog::common(object, message, button_prev, button_next, interaction), scope::SELF);
}

void context::on_dialog(character&                      me,
                        const fb::model::npc&           npc,
                        const std::string&              message,
                        const std::vector<std::string>& menus,
                        dialog::interaction             interaction)
{
    this->send(me, fb_resp::dialog::menu(npc, menus, message, interaction), scope::SELF);
}

void context::on_dialog(character&                  me,
                        const fb::model::npc&       npc,
                        const std::string&          message,
                        const std::vector<uint8_t>& item_slots,
                        dialog::interaction         interaction)
{
    this->send(me, fb_resp::dialog::slot(npc, item_slots, message, interaction), scope::SELF);
}

void context::on_dialog(character&                me,
                        const fb::model::npc&     npc,
                        const std::string&        message,
                        const dialog::item_pairs& pairs,
                        uint16_t                  pursuit,
                        dialog::interaction       interaction)
{
    this->send(me, fb_resp::dialog::item(npc, pairs, message, pursuit, interaction), scope::SELF);
}

void context::on_dialog(character&            me,
                        const fb::model::npc& npc,
                        const std::string&    message,
                        dialog::interaction   interaction)
{
    this->send(me, fb_resp::dialog::input(npc, message, interaction), scope::SELF);
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
    this->send(me, fb_resp::dialog::input_ext(npc, message, top, bottom, maxlen, prev, interaction), scope::SELF);
}

void context::on_trade_item(character& me, character& from, uint8_t index)
{
    bool mine = (&me == &from);
    this->send(me, fb_resp::trade::upload(from, index, mine), scope::SELF);
}

void context::on_option(character& me, CUSTOM_SETTING option, bool enabled)
{
    std::stringstream sstream;

    switch (option)
    {
    case CUSTOM_SETTING::WHISPER:
        sstream << "귓속말듣기  ";
        break;

    case CUSTOM_SETTING::GROUP:
    {
        auto group = me.group();
        if (group != nullptr)
        {
            if (group->members().size() == 1)
            {
                this->send(me, fb_resp::message("그룹 해체", MESSAGE_TYPE::STATE), scope::GROUP);
                group::destroy(*group);
            }
            else
            {
                auto              leader = group->leave(me);
                std::stringstream sstream;
                sstream << me.name() << "님 그룹 탈퇴";
                this->send(*leader, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
            }
        }

        sstream << "그룹허가    ";
        break;
    }

    case CUSTOM_SETTING::ROAR:
        sstream << "외치기듣기  ";
        break;

    case CUSTOM_SETTING::ROAR_WORLDS:
        sstream << "세계후      ";
        break;

    case CUSTOM_SETTING::MAGIC_EFFECT:
        sstream << "마법이펙트  ";
        break;

    case CUSTOM_SETTING::WEATHER_EFFECT:
        sstream << "날씨변화    ";
        break;

    case CUSTOM_SETTING::FIXED_MOVE:
        sstream << "고정이동    ";
        break;

    case CUSTOM_SETTING::TRADE:
        sstream << "교환가능    ";
        break;

    case CUSTOM_SETTING::FAST_MOVE:
        sstream << "빠른이동    ";
        break;

    case CUSTOM_SETTING::EFFECT_SOUND:
        sstream << "소리듣기    ";
        break;

    case CUSTOM_SETTING::PK:
        sstream << "PK보호      ";
        break;

    default:
        return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
    this->send(me, fb_resp::session::option(me), scope::SELF);
}

void context::on_level_up(character& me)
{
    this->send(me, fb_resp::object::effect(me, 0x02), scope::PIVOT);
}

void context::on_map_changed(object& me, map* before, map* after)
{
    if (after == nullptr)
        return;

    if (me.is(OBJECT_TYPE::CHARACTER) == false)
        return;

    auto& session = static_cast<character&>(me);
    this->send(session, fb_resp::session::id(session), scope::SELF);
    this->send(session, fb_resp::map::config(*after), scope::SELF);
    this->send(session, fb_resp::map::bgm(*after), scope::SELF);
    this->send(session, fb_resp::session::position(session), scope::SELF);
    this->send(session, fb_resp::session::show(session, session, false), scope::SELF);
    this->send(session, fb_resp::object::direction(session), scope::SELF);

    if (before == nullptr)
        this->save(session);
}

async::task<bool> context::on_transfer(character& me, map& map, const point16_t& position)
{
    fb::ostream parameter;
    parameter.write(me.name());

    auto& socket = static_cast<fb::socket<character>&>(me);
    auto  fd     = static_cast<uint32_t>(socket.native_handle());

    try
    {
        auto&& response =
            co_await this->post<fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer>(
                "internal",
                "/in-game/transfer",
                fb::protocol::internal::request::Transfer{fb::protocol::internal::Service::Game, map.model.host});

        if (response.code != fb::protocol::internal::TransferResult::Success)
            throw std::runtime_error("비바람이 휘몰아치고 있습니다.");

        auto session = socket.data();
        co_await session->map(nullptr);

        co_await this->save(*session);
        fb::ostream parameter;
        parameter.write_u32(me.id());
        parameter.write(session->name());
        parameter.write_u8(1);
        parameter.write_u16(map.model.id);
        parameter.write_u16(position.x);
        parameter.write_u16(position.y);
        this->transfer(socket, response.ip, response.port, fb::protocol::internal::services::GAME, parameter);

        co_return true;
    }
    catch (std::exception& e)
    {
        auto client = this->sockets[fd];
        if (client != nullptr)
        {
            auto message = e.what();
            auto session = client->data();
            session->refresh_map();
            this->on_notify(*session, message, MESSAGE_TYPE::STATE);
        }
        co_return false;
    }
    catch (boost::system::error_code& /*e*/)
    {
        auto client = this->sockets[fd];
        if (client != nullptr)
        {
            auto message = "비바람이 휘몰아치고 있습니다.";
            auto session = client->data();
            session->refresh_map();
            this->on_notify(*session, message, MESSAGE_TYPE::STATE);
        }

        co_return false;
    }
}

void context::on_item_get(character& me, const std::map<uint8_t, item*>& items)
{ }

void context::on_item_changed(character& me, const std::map<uint8_t, item*>& items)
{ }

void context::on_item_lost(character& me, const std::vector<uint8_t>& slots)
{ }

void context::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    this->send(me, fb_resp::item::remove(attr, index, 0), scope::SELF);
}

void context::on_item_update(character& me, uint8_t index)
{
    this->send(me, fb_resp::item::update(me, index), scope::SELF);
}

void context::on_item_swap(character& me, uint8_t src, uint8_t dst)
{ }