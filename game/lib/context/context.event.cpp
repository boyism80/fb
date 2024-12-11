#include <context.h>

using namespace fb::game;

async::task<void> context::on_create(object& me)
{
    co_return;
}

async::task<void> context::on_destroy(object& me)
{
    co_return;
}

async::task<void> context::on_chat(object& me, const std::string& message, bool shout)
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

    co_await this->send(me,
                        fb_resp::chat(me, sstream.str(), shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL),
                        shout ? scope::MAP : scope::PIVOT);
}

async::task<void> context::on_direction(object& me)
{
    co_await this->send(me, fb_resp::object::direction(me), scope::PIVOT, true);
}

async::task<void> context::on_show(object& me, bool light)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        co_await this->send(
            me,
            [&me, light](const auto& to) {
                return std::unique_ptr<fb::protocol::base::header>(
                    new fb_resp::session::show(static_cast<character&>(me), to, light));
            },
            scope::PIVOT);
    }
    else
    {
        co_await this->send(me, fb_resp::object::show(me), scope::PIVOT);
    }
}

async::task<void> context::on_show(object& me, object& you, bool light)
{
    if (you.is(OBJECT_TYPE::CHARACTER))
        co_await this->send(me, fb_resp::session::show(static_cast<character&>(you), me, light), scope::SELF);
    else
        co_await this->send(me, fb_resp::object::show(you), scope::SELF);
}

async::task<void> context::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        co_await this->send(me, fb_resp::object::hide(me), scope::PIVOT, true);
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        co_await this->send(me, fb_resp::life::die(static_cast<life&>(me)), scope::PIVOT, true);
        break;
    }
}

async::task<void> context::on_hide(object& me, object& you, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        co_await this->send(me, fb_resp::object::hide(you), scope::SELF);
        break;

    case DESTROY_TYPE::DEAD:
        if (you.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        co_await this->send(me, fb_resp::life::die(static_cast<life&>(you)), scope::SELF);
        break;
    }
}

async::task<void> context::on_move(object& me, const point16_t& before)
{
    co_await this->send(me, fb_resp::object::move(me, before), scope::PIVOT, true);
}

async::task<void> context::on_unbuff(object& me, buff& buff)
{
    if (buff.model.uncast.empty())
        co_return;

    auto thread = lua::get();
    if (thread == nullptr)
        co_return;
    thread->from(buff.model.uncast.c_str()).func("on_uncast").pushobject(me).pushobject(buff.model).resume(2);
    co_await this->send(me, fb_resp::spell::unbuff(buff), scope::SELF);
}

async::task<void> context::on_attack(life& me, object* you)
{
    switch (me.what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        co_await this->send(me,
                            fb_resp::session::action(static_cast<character&>(me), ACTION::ATTACK, DURATION::ATTACK),
                            scope::PIVOT);
        auto* weapon = static_cast<character&>(me).items.weapon();
        if (weapon != nullptr)
        {
            auto sound = weapon->based<fb::model::weapon>().sound;
            co_await this->send(me, fb_resp::object::sound(me, sound != 0 ? SOUND(sound) : SOUND::SWING), scope::PIVOT);
        }
    }
    break;

    case OBJECT_TYPE::MOB:
        co_await this->send(me, fb_resp::life::action(me, ACTION::ATTACK, DURATION::ATTACK), scope::PIVOT, true);
        break;
    }
}

async::task<void> context::on_hit(life& me, life& you, uint32_t damage, bool critical)
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
            co_return;
#endif

        auto* weapon = static_cast<character&>(me).items.weapon();
        if (weapon != nullptr)
            co_await this->send(me, fb_resp::object::sound(me, SOUND::DAMAGE), scope::PIVOT);

        you.hp_down(damage, &me, critical);
    }
    break;
    }
}

async::task<void> context::on_kill(life& me, life& you)
{
    co_return;
}

async::task<void> context::on_damaged(life& me, object* you, uint32_t damage, bool critical)
{
    co_await this->send(me, fb_resp::life::show_hp(me, damage, false), scope::PIVOT);
}

async::task<void> context::on_die(life& me, object* you)
{
    co_return;
}

async::task<void> context::on_heal_hp(life& me, uint32_t value, object* from)
{
    co_return;
}

async::task<void> context::on_heal_mp(life& me, uint32_t value, object* from)
{
    co_return;
}

async::task<void> context::on_hp(life& me, uint32_t before, uint32_t current)
{
    co_return;
}

async::task<void> context::on_mp(life& me, uint32_t before, uint32_t current)
{
    co_return;
}

async::task<void> context::on_action(character& me, ACTION action, DURATION duration, uint8_t sound)
{
    co_await this->send(me, fb_resp::session::action(me, action, duration), scope::PIVOT);
}

async::task<void> context::on_updated(character& me, STATE_LEVEL level)
{
    co_await this->send(me, fb_resp::session::state(me, level), scope::SELF);
}

async::task<void> context::on_money_changed(character& me, uint32_t value)
{
    co_return;
}

async::task<void> context::on_hold(character& me)
{
    co_await this->send(me, fb_resp::session::position(me), scope::SELF);
}

async::task<void> context::on_notify(character& me, const std::string& message, MESSAGE_TYPE type)
{
    co_await this->send(me, fb_resp::message(message, type), scope::SELF);
}

async::task<void> context::on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)
{
    co_await this->send(me, fb_resp::item::update_slot(me, parts), scope::SELF);
    co_await this->send(me, fb_resp::object::sound(me, SOUND::EQUIPMENT_ON), scope::PIVOT);

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
    co_await this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);

    sstream.str(std::string());
    sstream << "갑옷 강도  " << me.defensive_physical() << "  " << me.regenerative() << " S  "
            << me.defensive_magical();
    co_await this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
}

async::task<void> context::on_equipment_off(character& me, EQUIPMENT_PARTS parts, uint8_t index)
{
    co_await this->send(me, fb_resp::object::sound(me, SOUND::EQUIPMENT_OFF), scope::PIVOT);
}

async::task<void> context::on_item_active(character& me, item& item)
{
    auto thread = lua::get();
    if (thread == nullptr)
        co_return;

    thread->from(item.based<fb::model::item>().script_active.c_str())
        .func("on_active")
        .pushobject(me)
        .pushobject(item)
        .resume(2);
}

async::task<void> context::on_item_throws(character& me, item& item, const point16_t& to)
{
    if (me.position() != to)
        co_await this->send(me, fb_resp::session::throws(me, item, to), scope::PIVOT);
    else
        co_await this->send(me, fb_resp::session::action(me, ACTION::ATTACK, DURATION::THROW), scope::PIVOT);
}

async::task<void> context::on_spell_update(life& me, uint8_t index)
{
    co_await this->send(me, fb_resp::spell::update(me, index), scope::SELF);
}

async::task<void> context::on_spell_remove(life& me, uint8_t index)
{
    co_await this->send(me, fb_resp::spell::remove(me, index), scope::SELF);
}

async::task<void> context::on_trade_begin(character& me, character& you)
{
    co_await this->send(me, fb_resp::trade::dialog(you, this->model), scope::SELF);
}

async::task<void> context::on_trade_bundle(character& me)
{
    co_await this->send(me, fb_resp::trade::bundle(), scope::SELF);
}

async::task<void> context::on_trade_money(character& me, character& from)
{
    bool mine = (&me == &from);
    co_await this->send(me, fb_resp::trade::money(from, mine), scope::SELF);
}

async::task<void> context::on_trade_cancel(character& me, character& from)
{
    bool mine = (&me == &from);
    co_await this->send(
        me,
        fb_resp::trade::close(mine ? message::trade::CANCELLED_BY_ME : message::trade::CANCELLED_BY_PARTNER),
        scope::SELF);
}

async::task<void> context::on_trade_lock(character& me, bool mine)
{
    if (mine)
    {
        co_await this->send(me, fb_resp::trade::lock(), scope::SELF);
    }
    else
    {
        co_await this->send(me,
                            fb_resp::message(message::trade::NOTIFY_LOCK_TO_PARTNER, MESSAGE_TYPE::POPUP),
                            scope::SELF);
    }
}

async::task<void> context::on_trade_failed(character& me)
{
    co_await this->send(me, fb_resp::trade::close(message::trade::FAILED), scope::SELF);
}

async::task<void> context::on_trade_success(character& me)
{
    co_await this->send(me, fb_resp::trade::close(message::trade::SUCCESS), scope::SELF);
}

// new dialog
async::task<void> context::on_dialog(character&               me,
                                     const fb::model::object& object,
                                     const std::string&       message,
                                     bool                     button_prev,
                                     bool                     button_next,
                                     dialog::interaction      interaction)
{
    co_await this->send(me,
                        fb_resp::dialog::common(object, message, button_prev, button_next, interaction),
                        scope::SELF);
}

async::task<void> context::on_dialog(character&                      me,
                                     const fb::model::npc&           npc,
                                     const std::string&              message,
                                     const std::vector<std::string>& menus,
                                     dialog::interaction             interaction)
{
    co_await this->send(me, fb_resp::dialog::menu(npc, menus, message, interaction), scope::SELF);
}

async::task<void> context::on_dialog(character&                  me,
                                     const fb::model::npc&       npc,
                                     const std::string&          message,
                                     const std::vector<uint8_t>& item_slots,
                                     dialog::interaction         interaction)
{
    co_await this->send(me, fb_resp::dialog::slot(npc, item_slots, message, interaction), scope::SELF);
}

async::task<void> context::on_dialog(character&                me,
                                     const fb::model::npc&     npc,
                                     const std::string&        message,
                                     const dialog::item_pairs& pairs,
                                     uint16_t                  pursuit,
                                     dialog::interaction       interaction)
{
    co_await this->send(me, fb_resp::dialog::item(npc, pairs, message, pursuit, interaction), scope::SELF);
}

async::task<void> context::on_dialog(character&            me,
                                     const fb::model::npc& npc,
                                     const std::string&    message,
                                     dialog::interaction   interaction)
{
    co_await this->send(me, fb_resp::dialog::input(npc, message, interaction), scope::SELF);
}

async::task<void> context::on_dialog(character&            me,
                                     const fb::model::npc& npc,
                                     const std::string&    message,
                                     const std::string&    top,
                                     const std::string&    bottom,
                                     int                   maxlen,
                                     bool                  prev,
                                     dialog::interaction   interaction)
{
    co_await this->send(me,
                        fb_resp::dialog::input_ext(npc, message, top, bottom, maxlen, prev, interaction),
                        scope::SELF);
}

async::task<void> context::on_trade_item(character& me, character& from, uint8_t index)
{
    bool mine = (&me == &from);
    co_await this->send(me, fb_resp::trade::upload(from, index, mine), scope::SELF);
}

async::task<void> context::on_option(character& me, SETTING option, bool enabled)
{
    std::stringstream sstream;

    switch (option)
    {
    case SETTING::WHISPER:
        sstream << "귓속말듣기  ";
        break;

    case SETTING::GROUP:
    {
        auto group = me.group();
        if (group != nullptr)
        {
            // if (group->members().size() == 1)
            //{
            //     co_await this->send(me, fb_resp::message("그룹 해체", MESSAGE_TYPE::STATE), scope::GROUP);
            //     group::destroy(*group);
            // }
            // else
            //{
            //     auto              leader = group->leave(me);
            //     std::stringstream sstream;
            //     sstream << me.name() << "님 그룹 탈퇴";
            //     co_awaitthis->send(*leader, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
            // }
        }

        sstream << "그룹허가    ";
        break;
    }

    case SETTING::ROAR:
        sstream << "외치기듣기  ";
        break;

    case SETTING::ROAR_WORLDS:
        sstream << "세계후      ";
        break;

    case SETTING::MAGIC_EFFECT:
        sstream << "마법이펙트  ";
        break;

    case SETTING::WEATHER_EFFECT:
        sstream << "날씨변화    ";
        break;

    case SETTING::FIXED_MOVE:
        sstream << "고정이동    ";
        break;

    case SETTING::TRADE:
        sstream << "교환가능    ";
        break;

    case SETTING::FAST_MOVE:
        sstream << "빠른이동    ";
        break;

    case SETTING::EFFECT_SOUND:
        sstream << "소리듣기    ";
        break;

    case SETTING::PK_PROTECT:
        sstream << "PK보호      ";
        break;

    default:
        co_return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    co_await this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
    co_await this->send(me, fb_resp::session::option(me), scope::SELF);
}

async::task<void> context::on_level_up(character& me)
{
    co_await this->send(me, fb_resp::object::effect(me, 0x02), scope::PIVOT);
}

async::task<void> context::on_map_changed(object& me, map* before, map* after)
{
    if (after == nullptr)
        co_return;

    if (me.is(OBJECT_TYPE::CHARACTER) == false)
        co_return;

    auto& session = static_cast<character&>(me);
    co_await this->send(session, fb_resp::session::id(session), scope::SELF);
    co_await this->send(session, fb_resp::map::config(*after), scope::SELF);
    co_await this->send(session, fb_resp::map::bgm(*after), scope::SELF);
    co_await this->send(session, fb_resp::session::position(session), scope::SELF);
    co_await this->send(session, fb_resp::session::show(session, session, false), scope::SELF);
    co_await this->send(session, fb_resp::object::direction(session), scope::SELF);

    if (before == nullptr)
        co_await this->save(session);
}

async::task<bool> context::on_transfer(character& me, map& map, const point16_t& position)
{
    auto& socket = static_cast<fb::socket<character>&>(me);
    auto  fd     = static_cast<uint32_t>(socket.native_handle());
    auto  error  = std::string();
    try
    {
        auto&& response =
            co_await this->post<fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer>(
                "internal",
                "/in-game/transfer",
                fb::protocol::internal::request::Transfer{fb::protocol::internal::Service::Game,
                                                          map.model.host,
                                                          me.name(),
                                                          false});

        switch (static_cast<ERROR_CODE>(response.error))
        {
        case ERROR_CODE::NONE:
            break;

        case ERROR_CODE::SERVER_NOT_READY:
            throw std::runtime_error("비바람이 휘몰아치고 있습니다.");

        default:
            throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", response.error));
        }

        auto session = socket.data();
        std::ignore  = co_await session->map(nullptr);

        co_await this->save(*session);
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        writer.write<uint32_t>(me.id());
        writer.write(session->name());
        writer.write<uint8_t>(1);
        writer.write<uint16_t>(map.model.id);
        writer.write<uint16_t>(position.x);
        writer.write<uint16_t>(position.y);
        co_await this->transfer(socket, response.ip, response.port, fb::protocol::internal::Service::Game, stream);

        co_return true;
    }
    catch (std::exception& e)
    {
        error = e.what();
    }
    catch (boost::system::error_code& /*e*/)
    {
        error = "비바람이 휘몰아치고 있습니다.";
    }

    auto client = this->sockets[fd];
    if (client != nullptr)
    {
        auto session = client->data();
        co_await session->refresh_map();
        co_await this->on_notify(*session, error, MESSAGE_TYPE::STATE);
    }
    co_return false;
}

async::task<void> context::on_item_get(character& me, const item::container& items)
{
    co_return;
}

async::task<void> context::on_item_changed(character& me, const item::container& items)
{
    co_return;
}

async::task<void> context::on_item_lost(character& me, const std::vector<uint8_t>& slots)
{
    co_return;
}

async::task<void> context::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    co_await this->send(me, fb_resp::item::remove(attr, index, 0), scope::SELF);
}

async::task<void> context::on_item_update(character& me, uint8_t index)
{
    co_await this->send(me, fb_resp::item::update(me, index), scope::SELF);
}

async::task<void> context::on_item_swap(character& me, uint8_t src, uint8_t dst)
{
    co_return;
}