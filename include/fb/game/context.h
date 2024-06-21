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
#include <fb/core/format.h>
#include <fb/core/db.h>
#include <fb/core/acceptor.h>
#include <fb/game/session.h>
#include <fb/game/group.h>
#include <fb/game/listener.h>
#include <fb/game/query.h>
#include <fb/game/regex.h>
#include <fb/protocol/game.h>
#include <fb/protocol/internal.h>
#include <fb/game/model.h>

namespace fb { namespace game {

class context : public fb::acceptor<fb::game::session>, public fb::game::listener
{
public:
    LUA_PROTOTYPE

public:
    enum class scope { SELF, PIVOT, GROUP, MAP, WORLD };

public:
    struct command;

public:
    using command_fn        = std::function<fb::task<bool>(fb::game::session&, Json::Value&)>;
    using commands          = std::map<std::string, command>;
    using object_set        = std::map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using transfer_param    = fb::protocol::game::request::login::transfer_param;

private:
    commands                 _commands;
    std::mutex               _hash_mutex;
    object_set               _objects;
    maps                     _maps;
    fb::db::context<session> _db;
    tm*                      _time = fb::now();

public:
    fb::model::container    model;

public:
    context(boost::asio::io_context& context, uint16_t port);
    ~context();

private:
    uint32_t                elapsed_seconds(const daotk::mysql::datetime& datetime);
    std::string             elapsed_message(const daotk::mysql::datetime& datetime);
    fb::game::session*      find(const std::string& name);
    void                    bind_timer(const std::function<void(std::chrono::steady_clock::duration, std::thread::id)>& fn, const std::chrono::steady_clock::duration& duration);
    void                    bind_command(const std::string& cmd, const command& param);
    bool                    fetch_user(daotk::mysql::result& db_result, fb::game::session& session, const std::optional<transfer_param>& transfer);
    void                    fetch_item(daotk::mysql::result& db_result, fb::game::session& session);
    void                    fetch_spell(daotk::mysql::result& db_result, fb::game::session& session);

private:
    fb::task<void>          co_transfer(fb::game::session& me, fb::game::map& map, const point16_t& position, fb::awaiter<bool>* awaiter);

public:
    bool                    exists(const fb::game::object& object) const;

    template <typename T, typename... Args>
    T*                      make(Args&&... args)
    {
        return new T(*this, std::forward<Args>(args)...);
    }
    bool                    destroy(fb::game::object& obj);

public:
    void                    send(fb::game::object& object, const fb::protocol::base::header& header, context::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send(fb::game::object& object, const std::function<std::unique_ptr<fb::protocol::base::header>(const fb::game::object&)>& fn, context::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send(const fb::protocol::base::header& header, const fb::game::map& map, bool encrypt = true);
    void                    send(const fb::protocol::base::header& header, bool encrypt = true);
    void                    save(fb::game::session& session);
    fb::task<void>          save(fb::game::session& session, std::function<void(fb::game::session&)> fn);
    void                    save();
    void                    save(const std::function<void(fb::game::session&)>& fn);
    fb::awaiter<void>       co_save(fb::game::session& session);

public:
    fb::thread*             thread(const fb::game::map* map) const;
    uint8_t                 thread_index(const fb::game::map* map) const;
    fb::thread*             thread(const fb::game::object& obj) const;
    uint8_t                 thread_index(const fb::game::object& obj) const;
    const fb::thread*       current_thread() const;

protected:
    bool                    decrypt_policy(uint8_t cmd) const final;
    void                    handle_start() final;
    bool                    handle_connected(fb::socket<fb::game::session>& session) final;
    bool                    handle_disconnected(fb::socket<fb::game::session>& session) final;
    fb::game::session*      handle_accepted(fb::socket<fb::game::session>& socket) final;
    void                    handle_internal_connected() final;
    uint8_t                 handle_thread_index(fb::socket<fb::game::session>& socket) const final;

public:
    void                    handle_click_mob(fb::game::session& session, fb::game::mob& mob);
    void                    handle_click_npc(fb::game::session& session, fb::game::npc& npc);

public:
    fb::task<bool>          handle_in_message(fb::internal::socket<>&, const fb::protocol::internal::response::message&);
    fb::task<bool>          handle_in_logout(fb::internal::socket<>&, const fb::protocol::internal::response::logout&);
    fb::task<bool>          handle_in_shutdown(fb::internal::socket<>&, const fb::protocol::internal::response::shutdown&);

    // game event method
public:
    void                    handle_timer(uint64_t elapsed_milliseconds);

    // game event method
public:
    fb::task<bool>          handle_login(fb::socket<fb::game::session>&, const fb::protocol::game::request::login&);
    fb::task<bool>          handle_direction(fb::socket<fb::game::session>&, const fb::protocol::game::request::direction&);
    fb::task<bool>          handle_logout(fb::socket<fb::game::session>&, const fb::protocol::game::request::exit&);
    fb::task<bool>          handle_move(fb::socket<fb::game::session>&, const fb::protocol::game::request::move&);
    fb::task<bool>          handle_update_move(fb::socket<fb::game::session>&, const fb::protocol::game::request::update_move&);
    fb::task<bool>          handle_attack(fb::socket<fb::game::session>&, const fb::protocol::game::request::attack&);
    fb::task<bool>          handle_pickup(fb::socket<fb::game::session>&, const fb::protocol::game::request::pick_up&);
    fb::task<bool>          handle_emotion(fb::socket<fb::game::session>&, const fb::protocol::game::request::emotion&);
    fb::task<bool>          handle_update_map(fb::socket<fb::game::session>&, const fb::protocol::game::request::map::update&);
    fb::task<bool>          handle_refresh(fb::socket<fb::game::session>&, const fb::protocol::game::request::refresh&);
    fb::task<bool>          handle_active_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::active&);
    fb::task<bool>          handle_inactive_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::inactive&);
    fb::task<bool>          handle_drop_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::drop&);
    fb::task<bool>          handle_drop_cash(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::drop_cash&);
    fb::task<bool>          handle_front_info(fb::socket<fb::game::session>&, const fb::protocol::game::request::front_info&);
    fb::task<bool>          handle_self_info(fb::socket<fb::game::session>&, const fb::protocol::game::request::self_info&);
    fb::task<bool>          handle_option_changed(fb::socket<fb::game::session>&, const fb::protocol::game::request::change_option&);
    fb::task<bool>          handle_click_object(fb::socket<fb::game::session>&, const fb::protocol::game::request::click&);
    fb::task<bool>          handle_item_info(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::info&);
    fb::task<bool>          handle_itemmix(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::mix&);
    fb::task<bool>          handle_trade(fb::socket<fb::game::session>&, const fb::protocol::game::request::trade&);
    fb::task<bool>          handle_group(fb::socket<fb::game::session>&, const fb::protocol::game::request::group&);
    fb::task<bool>          handle_user_list(fb::socket<fb::game::session>&, const fb::protocol::game::request::user_list&);
    fb::task<bool>          handle_chat(fb::socket<fb::game::session>&, const fb::protocol::game::request::chat&);
    fb::task<bool>          handle_board(fb::socket<fb::game::session>&, const fb::protocol::game::request::board::board&);
    fb::task<bool>          handle_swap(fb::socket<fb::game::session>&, const fb::protocol::game::request::swap&);
    fb::task<bool>          handle_dialog(fb::socket<fb::game::session>&, const fb::protocol::game::request::dialog&);
    //fb::task<bool>          handle_dialog_1(fb::socket<fb::game::session>&, const fb::protocol::game::request::dialog1&);
    //fb::task<bool>          handle_dialog_2(fb::socket<fb::game::session>&, const fb::protocol::game::request::dialog2&);
    fb::task<bool>          handle_throw_item(fb::socket<fb::game::session>&, const fb::protocol::game::request::item::throws&);
    fb::task<bool>          handle_spell(fb::socket<fb::game::session>&, const fb::protocol::game::request::spell::use&);
    fb::task<bool>          handle_door(fb::socket<fb::game::session>&, const fb::protocol::game::request::door&);
    fb::task<bool>          handle_whisper(fb::socket<fb::game::session>&, const fb::protocol::game::request::whisper&);
    fb::task<bool>          handle_world(fb::socket<fb::game::session>&, const fb::protocol::game::request::map::world&);

public:
    void                    handle_mob_action(std::chrono::steady_clock::duration now, std::thread::id id);
    void                    handle_mob_respawn(std::chrono::steady_clock::duration now, std::thread::id id);
    void                    handle_buff_timer(std::chrono::steady_clock::duration now, std::thread::id id);
    void                    handle_save_timer(std::chrono::steady_clock::duration now, std::thread::id id);
    void                    handle_time(std::chrono::steady_clock::duration now, std::thread::id id);
    fb::task<bool>          handle_command(fb::game::session& session, const std::string& message);

public:
    fb::task<bool>          handle_command_map(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_sound(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_action(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_weather(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_bright(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_timer(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_effect(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_disguise(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_undisguise(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_mob(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_class(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_level(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_spell(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_item(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_world(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_script(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_hair(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_hair_color(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_armor_color(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_exit(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_tile(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_save(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_mapobj(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_randmap(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_npc(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_command_durability(fb::game::session& session, Json::Value& parameters);
    fb::task<bool>          handle_concurrency(fb::game::session& session, Json::Value& parameters);

public:
    // listener : object
    void                    on_create(fb::game::object& me) final;
    void                    on_destroy(fb::game::object& me) final;
    void                    on_chat(fb::game::object& me, const std::string& message, bool shout) final;
    void                    on_direction(fb::game::object& me) final;
    void                    on_show(fb::game::object& me, bool light) final;
    void                    on_show(fb::game::object& me, fb::game::object& you, bool light) final;
    void                    on_hide(fb::game::object& me) final;
    void                    on_hide(fb::game::object& me, fb::game::object& you) final;
    void                    on_move(fb::game::object& me, const point16_t& before) final;
    void                    on_unbuff(fb::game::object& me, fb::game::buff& buff) final;
    void                    on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after) final;

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
    void                    on_action(session& me, ACTION action, DURATION duration, uint8_t sound) final;
    void                    on_updated(session& me, fb::game::STATE_LEVEL level) final;
    void                    on_money_changed(session& me, uint32_t value) final;
    void                    on_notify(session& me, const std::string& message, MESSAGE_TYPE type) final;
    void                    on_option(session& me, fb::game::OPTION option, bool enabled) final;
    void                    on_level_up(session& me) final;
    void                    on_transfer(session& me, fb::game::map& map, const point16_t& position, fb::awaiter<bool>* awaiter = nullptr) final;
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
    void                    on_item_remove(session& me, uint8_t index, ITEM_DELETE_TYPE attr) final;
    void                    on_item_update(session& me, uint8_t index) final;
    void                    on_item_swap(session& me, uint8_t src, uint8_t dst) final;
    void                    on_equipment_on(session& me, item& item, EQUIPMENT_PARTS parts) final;
    void                    on_equipment_off(session& me, EQUIPMENT_PARTS parts, uint8_t index) final;
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
    void                    on_dialog(session& me, const fb::model::object& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::model::npc& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::model::npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::model::npc& npc, const std::string& message, const fb::game::dialog::item_pairs& pairs, uint16_t pursuit = 0xFFFF, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::model::npc& npc, const std::string& message,  fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void                    on_dialog(session& me, const fb::model::npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;

public:
    static int              builtin_seed(lua_State* lua);
    static int              builtin_sleep(lua_State* lua);
    static int              builtin_name2mob(lua_State* lua);
    static int              builtin_name2npc(lua_State* lua);
    static int              builtin_name2map(lua_State* lua);
    static int              builtin_name2item(lua_State* lua);
    static int              builtin_pursuit_sell(lua_State* lua);
    static int              builtin_pursuit_buy(lua_State* lua);
    static int              builtin_sell_price(lua_State* lua);
    static int              builtin_buy_price(lua_State* lua);
    static int              builtin_timer(lua_State* lua);
    static int              builtin_weather(lua_State* lua);
    static int              builtin_name_with(lua_State* lua);
    static int              builtin_assert_korean(lua_State* lua);
    static int              builtin_cp949(lua_State* lua);
};


struct context::command
{
public:
    context::command_fn     fn;
    bool                    admin;
};

} }

#endif // !__FB_GAME_H__
