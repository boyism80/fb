
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
#include <fb/core/db.h>
#include <fb/core/acceptor.h>
#include <fb/game/session.h>
#include <fb/game/group.h>
#include <fb/game/listener.h>
#include <fb/game/query.h>
#include <fb/protocol/game.h>
#include <fb/protocol/internal.h>

namespace fb { namespace game {

#pragma region message
#pragma endregion

#pragma region exceptions
#pragma endregion

class context : 
    public fb::acceptor<fb::game::session>, 
    public fb::game::listener
{
public:
    LUA_PROTOTYPE

#pragma region enum
public:
    enum class scope { SELF, PIVOT, GROUP, MAP, WORLD };
#pragma endregion

#pragma region forward nested declaration
public:
    struct command;
#pragma endregion

#pragma region type definition
public:
    using command_fn        = std::function<bool(fb::game::session&, Json::Value&)>;
    using commands          = std::map<std::string, command>;
    using object_set        = std::map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
#pragma endregion

#pragma region private field
private:
    commands                _commands;
    std::mutex              _hash_mutex;
    object_set              _objects;
    tm*                     _time = fb::now();
#pragma endregion

#pragma region constructor / destructor
public:
    context(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay, const INTERNAL_CONNECTION& internal_connection);
    ~context();
#pragma endregion

#pragma region private method
private:
    uint32_t                elapsed_seconds(const datetime& datetime);
    std::string             elapsed_message(const datetime& datetime);
    fb::game::session*      find(const std::string& name) const;
    void                    bind_timer(const std::function<void(std::chrono::steady_clock::duration, std::thread::id)>& fn, const std::chrono::steady_clock::duration& duration);
    void                    bind_command(const std::string& cmd, const command& param);
#pragma endregion

#pragma region public method
public:
    bool                    exists(const fb::game::object& object) const;

    template <typename T, typename... Args>
    T*                      make(const typename T::master* core, Args... args)
    {
        return new T(*this, core, args...);
    }
    bool                    destroy(fb::game::object& obj);

public:
    void                    send(fb::game::object& object, const fb::protocol::base::header& header, context::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send(fb::game::object& object, const std::function<std::unique_ptr<fb::protocol::base::header>(const fb::game::object&)>& fn, context::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send(const fb::protocol::base::header& header, const fb::game::map& map, bool encrypt = true);
    void                    send(const fb::protocol::base::header& header, bool encrypt = true);
    void                    save(fb::game::session& session);
    void                    save(fb::game::session& session, const std::function<void(fb::game::session&)>& fn);
    void                    save();
    void                    save(const std::function<void(fb::game::session&)>& fn);

public:
    fb::thread*             thread(const fb::game::map& map) const;
    uint8_t                 thread_index(const fb::game::map& map) const;
    fb::thread*             thread(const fb::game::object& obj) const;
    uint8_t                 thread_index(const fb::game::object& obj) const;
#pragma endregion

#pragma region handler method
protected:
    bool                    handle_connected(fb::socket<fb::game::session>& session) final;
    bool                    handle_disconnected(fb::socket<fb::game::session>& session) final;
    fb::game::session*      handle_accepted(fb::socket<fb::game::session>& socket) final;
    uint8_t                 handle_thread_index(fb::socket<fb::game::session>& socket) const final;
    void                    handle_exit() final;

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
    void                    handle_time(std::chrono::steady_clock::duration now, std::thread::id id);
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
#pragma endregion

#pragma region listener method
public:
    // listener : object
    void                    on_create(fb::game::object& me) final;
    void                    on_destroy(fb::game::object& me) final;
    void                    on_direction(fb::game::object& me) final;
    void                    on_show(fb::game::object& me, bool light) final;
    void                    on_show(fb::game::object& me, fb::game::object& you, bool light) final;
    void                    on_hide(fb::game::object& me) final;
    void                    on_hide(fb::game::object& me, fb::game::object& you) final;
    void                    on_move(fb::game::object& me) final;
    void                    on_unbuff(fb::game::object& me, fb::game::buff& buff) final;
    void                    on_enter(fb::game::object& me, fb::game::map& map, const point16_t& position) final;

    // listener : life
    void                    on_attack(life& me, object* you) final;
    void                    on_hit(life& me, life& you, uint32_t damage, bool critical) final;
    void                    on_kill(life& me, life& you) final;
    void                    on_damaged(life& me, object* you, uint32_t damage, bool critical) final;
    void                    on_die(life& me, object* you) final;

    void                    on_heal_hp(life& me, uint32_t value, fb::game::object* from) final;
    void                    on_heal_mp(life& me, uint32_t value, fb::game::object* from) final;
    void                    on_hp(life& me, uint32_t before, uint32_t current) final;
    void                    on_mp(life& me, uint32_t before, uint32_t current) final;

    // listener : session
    void                    on_attack(session& me, object* you) final;
    void                    on_hit(session& me, life& you, uint32_t damage, bool critical) final;
    void                    on_kill(session& me, life& you) final;
    void                    on_damaged(session& me, object* you, uint32_t damage, bool critical) final;
    void                    on_hold(session& me) final;
    void                    on_die(session& me, object* you) final;
    void                    on_action(session& me, action action, duration duration, uint8_t sound) final;
    void                    on_updated(session& me, fb::game::state_level level) final;
    void                    on_money_changed(session& me, uint32_t value) final;
    void                    on_notify(session& me, const std::string& message, message::type type) final;
    void                    on_option(session& me, fb::game::options option, bool enabled) final;
    void                    on_level_up(session& me) final;
    void                    on_warp(fb::game::session& me) final;
    void                    on_transfer(session& me, fb::game::map& map, const point16_t& position) final;
    void                    on_item_get(session& me, const std::map<uint8_t, fb::game::item*>& items) final;
    void                    on_item_changed(session& me, const std::map<uint8_t, fb::game::item*>& items) final;
    void                    on_item_lost(session& me, const std::vector<uint8_t>& slots) final;

    // listener : mob
    void                    on_attack(mob& me, object* you) final;
    void                    on_hit(mob& me, life& you, uint32_t damage, bool critical) final;
    void                    on_kill(mob& me, life& you) final;
    void                    on_damaged(mob& me, object* you, uint32_t damage, bool critical) final;
    void                    on_die(mob& me, object* you) final;

    // listener : item
    void                    on_item_remove(session& me, uint8_t index, item::delete_attr attr) final;
    void                    on_item_update(session& me, uint8_t index) final;
    void                    on_item_swap(session& me, uint8_t src, uint8_t dst) final;
    void                    on_equipment_on(session& me, item& item, equipment::slot slot) final;
    void                    on_equipment_off(session& me, equipment::slot slot, uint8_t index) final;
    void                    on_item_active(session& me, item& item) final;
    void                    on_item_throws(session& me, item& item, const point16_t& to) final;

    // listener : spell
    void                    on_spell_update(life& me, uint8_t index) final;
    void                    on_spell_remove(life& me, uint8_t index) final;


    // listener : trade
    void                    on_trade_begin(session& me, session& you) final;
    void                    on_trade_bundle(session& me) final;
    void                    on_trade_item(session& me, session& from, uint8_t index) final;
    void                    on_trade_money(session& me, session& from) final;
    void                    on_trade_cancel(session& me, session& from) final;
    void                    on_trade_lock(session& me, bool mine) final;
    void                    on_trade_failed(session& me) final;
    void                    on_trade_success(session& me) final;


    // listener : dialog
    void                    on_dialog(session& me, const object::master& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::vector<item::master*>& cores, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message,  fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::game::npc::master& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
#pragma endregion

#pragma region built-in method
public:
    static int              builtin_seed(lua_State* lua);
    static int              builtin_sleep(lua_State* lua);
    static int              builtin_name2mob(lua_State* lua);
    static int              builtin_name2npc(lua_State* lua);
    static int              builtin_name2map(lua_State* lua);
    static int              builtin_name2item(lua_State* lua);
    static int              builtin_timer(lua_State* lua);
    static int              builtin_weather(lua_State* lua);
#pragma endregion
};


struct context::command
{
public:
    context::command_fn     fn;
    bool                    admin;
};

} }

#endif // !__FB_GAME_H__
