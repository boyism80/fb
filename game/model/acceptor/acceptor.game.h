#ifndef __FB_GAME_H__
#define __FB_GAME_H__

#include <boost/algorithm/string/join.hpp>
#include <jsoncpp/json/json.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <unordered_set>
#include "module/db/db.h"
#include "module/acceptor/acceptor.h"
#include "module/config/config.h"
#include "model/session/session.h"
#include "model/group/group.h"
#include "model/clan/clan.h"
#include "model/life/life.h"
#include "model/listener/listener.h"
#include "protocol/game.h"
#include "protocol/internal.h"
#include "module/string/string.h"
#include "model/lua/lua.h"
#include "model/table/table.game.h"
#include "service/sql/sql.service.h"

namespace fb { namespace game {

#pragma region message
#pragma endregion

#pragma region exceptions
#pragma endregion

class acceptor : 
    public fb::acceptor<fb::game::session>, 
    public fb::game::listener
{
public:
    LUA_PROTOTYPE

public:
    enum scope { SELF, PIVOT, GROUP, MAP, WORLD };

public:
    typedef std::function<bool(fb::game::session&, Json::Value&)> command_fn;

    typedef struct __command
    {
    public:
        command_fn          fn;
        bool                admin;
    } command;

public:
    typedef std::map<std::string, command> command_dict;
    typedef std::unordered_set<const fb::game::object*> hash_set;

private:
    command_dict            _command_dict;
    std::mutex              _hash_mutex;
    hash_set                _hash_set;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay, const INTERNAL_CONNECTION& internal_connection);
    ~acceptor();

private:
    fb::game::session*      find(const std::string& name) const;
    void                    bind_timer(std::function<void(std::chrono::steady_clock::duration, std::thread::id)> fn, const std::chrono::steady_clock::duration& duration);
    void                    bind_command(const std::string& cmd, const command& param);

public:
    bool                    exists(const fb::game::object& object) const;

public:
    fb::game::session*      handle_accepted(fb::socket<fb::game::session>& socket);

public:
    void                    send(fb::game::object& object, const fb::protocol::base::header& header, acceptor::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send(fb::game::object& object, std::function<std::unique_ptr<fb::protocol::base::header>(const fb::game::object&)> fn, acceptor::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send(const fb::protocol::base::header& header, const fb::game::map& map, bool encrypt = true);
    void                    send(const fb::protocol::base::header& header, bool encrypt = true);
    void                    save();

protected:
    uint8_t                 handle_thread_index(fb::socket<fb::game::session>& socket) const;
    void                    handle_exit();

public:
    fb::thread*             thread(const fb::game::map& map) const;
    uint8_t                 thread_index(const fb::game::map& map) const;
    fb::thread*             thread(const fb::game::object& obj) const;
    uint8_t                 thread_index(const fb::game::object& obj) const;

public:
    void                    handle_click_mob(fb::game::session& session, fb::game::mob& mob);
    void                    handle_click_npc(fb::game::session& session, fb::game::npc& npc);

public:
    bool                    handle_in_transfer(fb::internal::socket<>&, const fb::protocol::internal::response::transfer&);
    bool                    handle_in_whisper(fb::internal::socket<>&, const fb::protocol::internal::response::whisper&);
    bool                    handle_in_message(fb::internal::socket<>&, const fb::protocol::internal::response::message&);
    bool                    handle_in_logout(fb::internal::socket<>&, const fb::protocol::internal::response::logout&);
    bool                    handle_in_shutdown(fb::internal::socket<>&, const fb::protocol::internal::response::shutdown&);

    // game event method
public:
    bool                    handle_connected(fb::socket<fb::game::session>& session);
    bool                    handle_disconnected(fb::socket<fb::game::session>& session);
    void                    handle_timer(uint64_t elapsed_milliseconds);

    // game event method
public:
    bool                    handle_login(fb::socket<fb::game::session>&, const fb::protocol::game::request::login&);
    bool                    handle_direction(fb::socket<fb::game::session>&, const fb::protocol::game::request::direction&);
    bool                    handle_logout(fb::socket<fb::game::session>&, const fb::protocol::game::request::exit&);
    bool                    handle_move(fb::socket<fb::game::session>&, const fb::protocol::game::request::move&);
    bool                    handle_update_move(fb::socket<fb::game::session>&, const fb::protocol::game::request::update_move&);
    bool                    handle_attack(fb::socket<fb::game::session>&, const fb::protocol::game::request::attack&);
    bool                    handle_pickup(fb::socket<fb::game::session>&, const fb::protocol::game::request::pick_up&);
    bool                    handle_emotion(fb::socket<fb::game::session>&, const fb::protocol::game::request::emotion&);
    bool                    handle_update_map(fb::socket<fb::game::session>&, const fb::protocol::game::request::map::update&);
    bool                    handle_refresh(fb::socket<fb::game::session>&, const fb::protocol::game::request::refresh&);
    bool                    handle_active_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::active&);
    bool                    handle_inactive_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::inactive&);
    bool                    handle_drop_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::drop&);
    bool                    handle_drop_cash(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::drop_cash&);
    bool                    handle_front_info(fb::socket<fb::game::session>&, const fb::protocol::game::request::front_info&);
    bool                    handle_self_info(fb::socket<fb::game::session>&, const fb::protocol::game::request::self_info&);
    bool                    handle_option_changed(fb::socket<fb::game::session>&, const fb::protocol::game::request::change_option&);
    bool                    handle_click_object(fb::socket<fb::game::session>&, const fb::protocol::game::request::click&);
    bool                    handle_item_info(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::info&);
    bool                    handle_itemmix(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::mix&);
    bool                    handle_trade(fb::socket<fb::game::session>&, const fb::protocol::game::request::trade&);
    bool                    handle_group(fb::socket<fb::game::session>&, const fb::protocol::game::request::group&);
    bool                    handle_user_list(fb::socket<fb::game::session>&, const fb::protocol::game::request::user_list&);
    bool                    handle_chat(fb::socket<fb::game::session>&, const fb::protocol::game::request::chat&);
    bool                    handle_board(fb::socket<fb::game::session>&, const fb::protocol::game::request::board::board&);
    bool                    handle_swap(fb::socket<fb::game::session>&, const fb::protocol::game::request::swap&);
    bool                    handle_dialog(fb::socket<fb::game::session>&, const fb::protocol::game::request::dialog&);
    bool                    handle_throw_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::throws&);
    bool                    handle_spell(fb::socket<fb::game::session>&, const fb::protocol::game::request::spell::use&);
    bool                    handle_door(fb::socket<fb::game::session>&, const fb::protocol::game::request::door&);
    bool                    handle_whisper(fb::socket<fb::game::session>&, const fb::protocol::game::request::whisper&);
    bool                    handle_world(fb::socket<fb::game::session>&, const fb::protocol::game::request::map::world&);

public:
    void                    handle_mob_action(std::chrono::steady_clock::duration now, std::thread::id id);
    void                    handle_mob_respawn(std::chrono::steady_clock::duration now, std::thread::id id);
    void                    handle_buff_timer(std::chrono::steady_clock::duration now, std::thread::id id);
    void                    handle_save_timer(std::chrono::steady_clock::duration now, std::thread::id id);
    bool                    handle_command(fb::game::session& session, const std::string& message);

public:
    bool                    handle_command_map(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_sound(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_action(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_weather(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_bright(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_timer(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_effect(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_disguise(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_undisguise(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_mob(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_class(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_level(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_spell(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_item(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_world(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_script(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_hair(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_hair_color(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_armor_color(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_exit(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_tile(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_save(fb::game::session& session, Json::Value& parameters);
    bool                    handle_command_mapobj(fb::game::session& session, Json::Value& parameters);

public:
    // listener : object
    void                    on_create(fb::game::object& me);
    void                    on_destroy(fb::game::object& me);
    void                    on_direction(fb::game::object& me);
    void                    on_show(fb::game::object& me, bool light);
    void                    on_show(fb::game::object& me, fb::game::object& you, bool light);
    void                    on_hide(fb::game::object& me);
    void                    on_hide(fb::game::object& me, fb::game::object& you);
    void                    on_move(fb::game::object& me);
    void                    on_unbuff(fb::game::object& me, fb::game::buff& buff);
    void                    on_enter(fb::game::object& me, fb::game::map& map, const point16_t& position);

    // listener : life
    void                    on_attack(life& me, object* you);
    void                    on_hit(life& me, life& you, uint32_t damage, bool critical);
    void                    on_kill(life& me, life& you);
    void                    on_damaged(life& me, object* you, uint32_t damage, bool critical);
    void                    on_die(life& me, object* you);

    void                    on_heal_hp(life& me, uint32_t value, fb::game::object* from);
    void                    on_heal_mp(life& me, uint32_t value, fb::game::object* from);
    void                    on_hp(life& me, uint32_t before, uint32_t current);
    void                    on_mp(life& me, uint32_t before, uint32_t current);

    // listener : session
    void                    on_attack(session& me, object* you);
    void                    on_hit(session& me, life& you, uint32_t damage, bool critical);
    void                    on_kill(session& me, life& you);
    void                    on_damaged(session& me, object* you, uint32_t damage, bool critical);
    void                    on_hold(session& me);
    void                    on_die(session& me, object* you);
    void                    on_action(session& me, action action, duration duration, uint8_t sound);
    void                    on_updated(session& me, fb::game::state_level level);
    void                    on_money_changed(session& me, uint32_t value);
    void                    on_notify(session& me, const std::string& message, message::type type);
    void                    on_option(session& me, fb::game::options option, bool enabled);
    void                    on_level_up(session& me);
    void                    on_warp(fb::game::session& me);
    void                    on_transfer(session& me, fb::game::map& map, const point16_t& position);
    void                    on_item_get(session& me, const std::map<uint8_t, fb::game::item*>& items);
    void                    on_item_changed(session& me, const std::map<uint8_t, fb::game::item*>& items);
    void                    on_item_lost(session& me, const std::vector<uint8_t>& slots);

    // listener : mob
    void                    on_attack(mob& me, object* you);
    void                    on_hit(mob& me, life& you, uint32_t damage, bool critical);
    void                    on_kill(mob& me, life& you);
    void                    on_damaged(mob& me, object* you, uint32_t damage, bool critical);
    void                    on_die(mob& me, object* you);

    // listener : item
    void                    on_item_remove(session& me, uint8_t index, item::delete_attr attr);
    void                    on_item_update(session& me, uint8_t index);
    void                    on_item_swap(session& me, uint8_t src, uint8_t dst);
    void                    on_equipment_on(session& me, item& item, equipment::slot slot);
    void                    on_equipment_off(session& me, equipment::slot slot, uint8_t index);
    void                    on_item_active(session& me, item& item);
    void                    on_item_throws(session& me, item& item, const point16_t& to);

    // listener : spell
    void                    on_spell_update(life& me, uint8_t index);
    void                    on_spell_remove(life& me, uint8_t index);


    // listener : trade
    void                    on_trade_begin(session& me, session& you);
    void                    on_trade_bundle(session& me);
    void                    on_trade_item(session& me, session& from, uint8_t index);
    void                    on_trade_money(session& me, session& from);
    void                    on_trade_cancel(session& me, session& from);
    void                    on_trade_lock(session& me, bool mine);
    void                    on_trade_failed(session& me);
    void                    on_trade_success(session& me);


    // listener : dialog
    void                    on_dialog(session& me, const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message,  fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);

    // listener : game
    void                    on_save(session& me);
    void                    on_save(session& me, std::function<void(fb::game::session&)> fn);


    // built-in method
public:
    static int              builtin_seed(lua_State* lua);
    static int              builtin_sleep(lua_State* lua);
    static int              builtin_name2mob(lua_State* lua);
    static int              builtin_name2npc(lua_State* lua);
    static int              builtin_name2map(lua_State* lua);
    static int              builtin_name2item(lua_State* lua);
    static int              builtin_timer(lua_State* lua);
    static int              builtin_weather(lua_State* lua);
};

} }

#endif // !__FB_GAME_H__
