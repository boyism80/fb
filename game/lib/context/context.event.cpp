#include <fb/game/context.h>

using namespace fb::game;

void context::on_create(object& me)
{
    return;
}

void context::on_destroy(object& me)
{
    return;
}

void context::on_chat(object& me, const std::string& message, CHAT_TYPE chat_type)
{
    if (me.is(OBJECT_TYPE::ITEM))
        return;

    auto sstream = std::stringstream();
    switch (chat_type)
    {
    case CHAT_TYPE::SHOUT:
        sstream << me.name() << "! " << message;
        break;

    default:
        sstream << me.name() << ": " << message;
        break;
    }

    auto scp = scope::PIVOT;
    switch (chat_type)
    {
    case CHAT_TYPE::SHOUT:
        scp = scope::MAP;
        break;

    case CHAT_TYPE::BLUE:
    case CHAT_TYPE::LIGHT_BLUE:
        scp = scope::WORLD;
        break;

    default:
        scp = scope::PIVOT;
        break;
    }
    this->send(me, fb_resp::chat(me, sstream.str(), chat_type), scp);
}

void context::on_direction(object& me)
{
    this->send(me, fb_resp::direction(me), scope::PIVOT, true);
}

void context::on_show(object& me, bool light)
{
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        this->send(
            me,
            [&me, light](const auto& to) {
                return std::unique_ptr<fb::protocol::header>(
                    new fb_resp::update_external(static_cast<character&>(me), to, light));
            },
            scope::PIVOT);
    }
    else
    {
        this->send(me, fb_resp::update(me), scope::PIVOT);
    }
}

void context::on_show(object& me, object& you, bool light)
{
    if (you.is(OBJECT_TYPE::CHARACTER))
        this->send(me, fb_resp::update_external(static_cast<character&>(you), me, light), scope::SELF);
    else
        this->send(me, fb_resp::update(you), scope::SELF);
}

void context::on_hide(object& me, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        this->send(me, fb_resp::hide(me), scope::PIVOT, true);
        break;

    case DESTROY_TYPE::DEAD:
        if (me.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->send(me, fb_resp::die(static_cast<life&>(me)), scope::PIVOT, true);
        break;
    }
}

void context::on_hide(object& me, object& you, DESTROY_TYPE destroy_type)
{
    switch (destroy_type)
    {
    case DESTROY_TYPE::DEFAULT:
        this->send(me, fb_resp::hide(you), scope::SELF);
        break;

    case DESTROY_TYPE::DEAD:
        if (you.is(OBJECT_TYPE::LIFE) == false)
            throw std::runtime_error("object must be life type");

        this->send(me, fb_resp::die(static_cast<life&>(you)), scope::SELF);
        break;
    }
}

void context::on_move(object& me, const point16_t& before)
{
    this->send(me, fb_resp::move(me, before), scope::PIVOT, true);
}

void context::on_unbuff(object& me, buff& buff)
{
    if (buff.model.uncast.empty())
        return;

    auto thread = lua::get();
    if (thread == nullptr)
        return;
    thread->from(buff.model.uncast.c_str()).func("on_uncast").pushobject(me).pushobject(buff.model).resume(2);
    this->send(me, fb_resp::spell_unbuff(buff), scope::SELF);
}

void context::on_attack(life& me, object* you)
{
    switch (me.what())
    {
    case OBJECT_TYPE::CHARACTER:
    {
        this->send(me, fb_resp::action(me, ACTION::ATTACK, DURATION::ATTACK), scope::PIVOT);
        auto* weapon = static_cast<character&>(me).items.weapon();
        if (weapon != nullptr)
        {
            auto sound = weapon->based<fb::model::weapon>().sound;
            this->send(me, fb_resp::sound(me, sound != 0 ? SOUND(sound) : SOUND::SWING), scope::PIVOT);
        }
    }
    break;

    case OBJECT_TYPE::MOB:
        this->send(me, fb_resp::action(me, ACTION::ATTACK, DURATION::ATTACK), scope::PIVOT, true);
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
            this->send(me, fb_resp::sound(me, SOUND::DAMAGE), scope::PIVOT);

        you.hp_down(damage, &me, critical);
    }
    break;
    }
}

void context::on_kill(life& me, life& you)
{
    return;
}

void context::on_damaged(life& me, object* you, uint32_t damage, bool critical)
{
    this->send(me, fb_resp::update_hp(me, damage, false), scope::PIVOT);
}

void context::on_die(life& me, object* you)
{
    return;
}

void context::on_heal_hp(life& me, uint32_t value, object* from)
{
    return;
}

void context::on_heal_mp(life& me, uint32_t value, object* from)
{
    return;
}

void context::on_hp(life& me, uint32_t before, uint32_t current)
{
    return;
}

void context::on_mp(life& me, uint32_t before, uint32_t current)
{
    return;
}

void context::on_action(character& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->send(me, fb_resp::action(me, action, duration), scope::PIVOT);
}

void context::on_updated(character& me, STATE_LEVEL level)
{
    this->send(me, fb_resp::update_internal(me, level), scope::SELF);
}

void context::on_money_changed(character& me, uint32_t value)
{
    return;
}

void context::on_hold(character& me)
{
    this->send(me, fb_resp::position(me), scope::SELF);
}

void context::on_message(character& me, const std::string& message, MESSAGE_TYPE type)
{
    this->send(me, fb_resp::message(message, type), scope::SELF);
}

void context::on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)
{
    this->send(me, fb_resp::item_update_slot(me, parts), scope::SELF);
    this->send(me, fb_resp::sound(me, SOUND::EQUIPMENT_ON), scope::PIVOT);

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
    this->send(me, fb_resp::sound(me, SOUND::EQUIPMENT_OFF), scope::PIVOT);
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
        this->send(me, fb_resp::item_throws(me, item, to), scope::PIVOT);
    else
        this->send(me, fb_resp::action(me, ACTION::ATTACK, DURATION::THROW), scope::PIVOT);
}

void context::on_spell_update(life& me, uint8_t index)
{
    this->send(me, fb_resp::spell_update(me, index), scope::SELF);
}

void context::on_spell_remove(life& me, uint8_t index)
{
    this->send(me, fb_resp::spell_remove(me, index), scope::SELF);
}

void context::on_trade_begin(character& me, character& you)
{
    this->send(me, fb_resp::trade_dialog(you, this->model), scope::SELF);
}

void context::on_trade_bundle(character& me)
{
    this->send(me, fb_resp::trade_bundle(), scope::SELF);
}

void context::on_trade_money(character& me, character& from)
{
    bool mine = (&me == &from);
    this->send(me, fb_resp::trade_money(from, mine), scope::SELF);
}

void context::on_trade_cancel(character& me, character& from)
{
    bool mine = (&me == &from);
    this->send(me,
               fb_resp::trade_close(mine ? message::trade::CANCELLED_BY_ME : message::trade::CANCELLED_BY_PARTNER),
               scope::SELF);
}

void context::on_trade_lock(character& me, bool mine)
{
    if (mine)
    {
        this->send(me, fb_resp::trade_lock(), scope::SELF);
    }
    else
    {
        this->send(me, fb_resp::message(message::trade::NOTIFY_LOCK_TO_PARTNER, MESSAGE_TYPE::POPUP), scope::SELF);
    }
}

void context::on_trade_failed(character& me)
{
    this->send(me, fb_resp::trade_close(message::trade::FAILED), scope::SELF);
}

void context::on_trade_success(character& me)
{
    this->send(me, fb_resp::trade_close(message::trade::SUCCESS), scope::SELF);
}

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

void context::on_trade_item(character& me, character& from, uint8_t index)
{
    bool mine = (&me == &from);
    this->send(me, fb_resp::trade_upload(from, index, mine), scope::SELF);
}

void context::on_option(character& me, SETTING option, bool enabled)
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
            //     this->send(me, fb_resp::message("그룹 해체", MESSAGE_TYPE::STATE), scope::GROUP);
            //     group::destroy(*group);
            // }
            // else
            //{
            //     auto              leader = group->leave(me);
            //     std::stringstream sstream;
            //     sstream << me.name() << "님 그룹 탈퇴";
            //     his->send(*leader, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::GROUP);
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
        return;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    this->send(me, fb_resp::message(sstream.str(), MESSAGE_TYPE::STATE), scope::SELF);
    this->send(me, fb_resp::option(me), scope::SELF);
}

void context::on_level_up(character& me)
{
    this->send(me, fb_resp::effect(me, 0x02), scope::PIVOT);
}

void context::on_map_changed(object& me, map* before, map* after)
{
    if (after == nullptr)
        return;

    if (me.is(OBJECT_TYPE::CHARACTER) == false)
        return;

    auto& ch = static_cast<character&>(me);
    this->send(ch, fb_resp::id(ch), scope::SELF);
    this->send(ch, fb_resp::map_config(*after), scope::SELF);
    this->send(ch, fb_resp::map_bgm(*after), scope::SELF);
    this->send(ch, fb_resp::position(ch), scope::SELF);
    this->send(ch, fb_resp::update_external(ch, ch, false), scope::SELF);
    this->send(ch, fb_resp::direction(ch), scope::SELF);

    if (before == nullptr)
        this->save(ch);
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

        auto ch     = socket.data();
        std::ignore = co_await ch->map(nullptr);

        this->save(*ch);
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        writer.write<uint32_t>(me.id());
        writer.write<std::string>(ch->name());
        writer.write<uint8_t>(1);
        writer.write<uint16_t>(map.model.id);
        writer.write<uint16_t>(position.x);
        writer.write<uint16_t>(position.y);
        std::ignore = this->transfer(socket, response.ip, response.port, fb::protocol::internal::Service::Game, stream);
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

    auto ch = this->_sockets.template lock<character*>([fd](auto& container) -> character* {
        if (container.contains(fd))
            return container.at(fd)->data();

        return nullptr;
    });
    if (ch != nullptr)
    {
        ch->refresh_map();
        this->on_message(*ch, error, MESSAGE_TYPE::STATE);
    }
    co_return false;
}

void context::on_item_get(character& me, const item::container& items)
{
    return;
}

void context::on_item_changed(character& me, const item::container& items)
{
    return;
}

void context::on_item_lost(character& me, const std::vector<uint8_t>& slots)
{
    return;
}

void context::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    this->send(me, fb_resp::item_remove(attr, index, 0), scope::SELF);
}

void context::on_item_update(character& me, uint8_t index)
{
    this->send(me, fb_resp::item_update(me, index), scope::SELF);
}

void context::on_item_swap(character& me, uint8_t src, uint8_t dst)
{
    return;
}