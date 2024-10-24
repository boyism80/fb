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
#include <format>
#include <fb/acceptor.h>
#include <character.h>
#include <group.h>
#include <listener.h>
#include <regex.h>
#include <fb/protocol/game.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <model.h>
#include <fb/amqp.h>

using namespace fb::protocol::internal;

namespace fb { namespace game {

class context : public fb::acceptor<fb::game::character>, public fb::game::listener
{
public:
    LUA_PROTOTYPE

public:
    enum class scope
    {
        SELF,
        PIVOT,
        GROUP,
        MAP,
        WORLD
    };

public:
    struct command;

public:
    using command_fn      = std::function<async::task<bool>(fb::game::character&, Json::Value&)>;
    using commands        = std::map<std::string, command>;
    using object_set      = std::map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using transfer_param  = fb::protocol::game::request::login::transfer_param;
    using rezen_container = std::vector<fb::game::rezen>;

private:
    commands                          _commands;
    datetime                          _time;
    std::unique_ptr<fb::amqp::socket> _amqp;
    std::unique_ptr<std::thread>      _amqp_thread;

public:
    fb::model::model model;
    fb::game::maps   maps;
    rezen_container  rezen;

public:
    context(boost::asio::io_context& context, uint16_t port);
    context(const context&) = delete;
    ~context();

private:
    std::string          elapsed_message(const std::string& datetime);
    fb::game::character* find(const std::string& name);
    void                 bind_timer(const std::function<void(const datetime&, std::thread::id)>& fn, const std::chrono::steady_clock::duration& duration);
    void                 bind_command(const std::string& cmd, const command& param);
    bool                 init_ch(const fb::protocol::db::Character& response, fb::game::character& session, const std::optional<transfer_param>& transfer);
    void                 init_items(const std::vector<fb::protocol::db::Item>& response, fb::game::character& session);
    void                 init_spells(const std::vector<fb::protocol::db::Spell>& response, fb::game::character& session);

public:
    template <typename T, typename... Args>
    T* make(Args&&... args)
    {
        return new T(*this, std::forward<Args>(args)...);
    }
    template <typename T>
    async::task<void> destroy(T& obj, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)
    {
        if constexpr (std::is_same_v<T, fb::game::object>)
        {
            co_await obj.map(nullptr, destroy_type);
        }
        delete &obj;
        co_return;
    }

public:
    void              send(fb::game::object& object, const fb::protocol::base::header& header, context::scope scope, bool exclude_self = false, bool encrypt = true);
    void              send(fb::game::object&                                                                          object,
                           const std::function<std::unique_ptr<fb::protocol::base::header>(const fb::game::object&)>& fn,
                           context::scope                                                                             scope,
                           bool                                                                                       exclude_self = false,
                           bool                                                                                       encrypt      = true);
    void              send(const fb::protocol::base::header& header, const fb::game::map& map, bool encrypt = true);
    void              send(const fb::protocol::base::header& header, bool encrypt = true);
    async::task<void> save(fb::game::character& session);

public:
    fb::thread*       thread(const fb::game::map* map) const;
    uint8_t           thread_index(const fb::game::map* map) const;
    fb::thread*       thread(const fb::game::object& obj) const;
    uint8_t           thread_index(const fb::game::object& obj) const;
    const fb::thread* current_thread() const;
    void              amqp_thread();

protected:
    bool                 decrypt_policy(uint8_t cmd) const final;
    async::task<void>    handle_start() final;
    bool                 handle_connected(fb::socket<fb::game::character>& session) final;
    async::task<bool>    handle_disconnected(fb::socket<fb::game::character>& session) final;
    fb::game::character* handle_accepted(fb::socket<fb::game::character>& socket) final;
    // async::task<void>       handle_internal_connected() final;
    uint8_t handle_thread_index(fb::socket<fb::game::character>& socket) const final;

    // for heart-beat

protected:
    uint8_t id() const
    {
        return fb::config::get()["id"].asUInt();
    }
    Service service() const
    {
        return Service::Game;
    }

public:
    void handle_click_mob(fb::game::character& session, fb::game::mob& mob);
    void handle_click_npc(fb::game::character& session, fb::game::npc& npc);

    // game event method

public:
    void handle_timer(uint64_t elapsed_milliseconds);

    // game event method

public:
    async::task<bool> handle_login(fb::socket<fb::game::character>&, const fb::protocol::game::request::login&);
    async::task<bool> handle_direction(fb::socket<fb::game::character>&, const fb::protocol::game::request::direction&);
    async::task<bool> handle_logout(fb::socket<fb::game::character>&, const fb::protocol::game::request::exit&);
    async::task<bool> handle_move(fb::socket<fb::game::character>&, const fb::protocol::game::request::move&);
    async::task<bool> handle_update_move(fb::socket<fb::game::character>&, const fb::protocol::game::request::update_move&);
    async::task<bool> handle_attack(fb::socket<fb::game::character>&, const fb::protocol::game::request::attack&);
    async::task<bool> handle_pickup(fb::socket<fb::game::character>&, const fb::protocol::game::request::pick_up&);
    async::task<bool> handle_emotion(fb::socket<fb::game::character>&, const fb::protocol::game::request::emotion&);
    async::task<bool> handle_update_map(fb::socket<fb::game::character>&, const fb::protocol::game::request::map::update&);
    async::task<bool> handle_refresh(fb::socket<fb::game::character>&, const fb::protocol::game::request::refresh&);
    async::task<bool> handle_active_item(fb::socket<fb::game::character>&, const fb::protocol::game::request::item::active&);
    async::task<bool> handle_inactive_item(fb::socket<fb::game::character>&, const fb::protocol::game::request::item::inactive&);
    async::task<bool> handle_drop_item(fb::socket<fb::game::character>&, const fb::protocol::game::request::item::drop&);
    async::task<bool> handle_drop_cash(fb::socket<fb::game::character>&, const fb::protocol::game::request::item::drop_cash&);
    async::task<bool> handle_front_info(fb::socket<fb::game::character>&, const fb::protocol::game::request::front_info&);
    async::task<bool> handle_self_info(fb::socket<fb::game::character>&, const fb::protocol::game::request::self_info&);
    async::task<bool> handle_option_changed(fb::socket<fb::game::character>&, const fb::protocol::game::request::change_option&);
    async::task<bool> handle_click_object(fb::socket<fb::game::character>&, const fb::protocol::game::request::click&);
    async::task<bool> handle_item_info(fb::socket<fb::game::character>&, const fb::protocol::game::request::item::info&);
    async::task<bool> handle_itemmix(fb::socket<fb::game::character>&, const fb::protocol::game::request::item::mix&);
    async::task<bool> handle_trade(fb::socket<fb::game::character>&, const fb::protocol::game::request::trade&);
    async::task<bool> handle_group(fb::socket<fb::game::character>&, const fb::protocol::game::request::group&);
    async::task<bool> handle_user_list(fb::socket<fb::game::character>&, const fb::protocol::game::request::user_list&);
    async::task<bool> handle_chat(fb::socket<fb::game::character>&, const fb::protocol::game::request::chat&);
    async::task<bool> handle_board(fb::socket<fb::game::character>&, const fb::protocol::game::request::board::board&);
    async::task<bool> handle_swap(fb::socket<fb::game::character>&, const fb::protocol::game::request::swap&);
    async::task<bool> handle_dialog(fb::socket<fb::game::character>&, const fb::protocol::game::request::dialog&);
    // async::task<bool>       handle_dialog_1(fb::socket<fb::game::character>&, const fb::protocol::game::request::dialog1&);
    // async::task<bool>       handle_dialog_2(fb::socket<fb::game::character>&, const fb::protocol::game::request::dialog2&);
    async::task<bool> handle_throw_item(fb::socket<fb::game::character>&, const fb::protocol::game::request::item::throws&);
    async::task<bool> handle_spell(fb::socket<fb::game::character>&, const fb::protocol::game::request::spell::use&);
    async::task<bool> handle_door(fb::socket<fb::game::character>&, const fb::protocol::game::request::door&);
    async::task<bool> handle_whisper(fb::socket<fb::game::character>&, const fb::protocol::game::request::whisper&);
    async::task<bool> handle_world(fb::socket<fb::game::character>&, const fb::protocol::game::request::map::world&);

public:
    void              handle_mob_action(const datetime& now, std::thread::id id);
    void              handle_mob_respawn(const datetime& now, std::thread::id id);
    void              handle_buff_timer(const datetime& now, std::thread::id id);
    void              handle_save_timer(const datetime& now, std::thread::id id);
    void              handle_time(const datetime& now, std::thread::id id);
    async::task<bool> handle_command(fb::game::character& session, const std::string& message);

public:
    async::task<bool> handle_command_map(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_sound(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_action(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_weather(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_bright(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_timer(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_effect(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_disguise(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_undisguise(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_mob(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_class(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_level(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_spell(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_item(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_world(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_script(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_hair(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_hair_color(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_armor_color(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_exit(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_tile(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_save(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_mapobj(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_randmap(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_npc(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_durability(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_concurrency(fb::game::character& session, Json::Value& parameters);
    async::task<bool> handle_command_sleep(fb::game::character& session, Json::Value& parameters);

public:
    // listener : object
    void on_create(fb::game::object& me) final;
    void on_destroy(fb::game::object& me) final;
    void on_chat(fb::game::object& me, const std::string& message, bool shout) final;
    void on_direction(fb::game::object& me) final;
    void on_show(fb::game::object& me, bool light) final;
    void on_show(fb::game::object& me, fb::game::object& you, bool light) final;
    void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) final;
    void on_hide(fb::game::object& me, fb::game::object& you, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) final;
    void on_move(fb::game::object& me, const point16_t& before) final;
    void on_unbuff(fb::game::object& me, fb::game::buff& buff) final;
    void on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after) final;

    // listener : life
    void on_attack(life& me, object* you) final;
    void on_hit(life& me, life& you, uint32_t damage, bool critical) final;
    void on_kill(life& me, life& you) final;
    void on_damaged(life& me, object* you, uint32_t damage, bool critical) final;
    void on_die(life& me, object* you) final;

    void on_heal_hp(life& me, uint32_t value, fb::game::object* from) final;
    void on_heal_mp(life& me, uint32_t value, fb::game::object* from) final;
    void on_hp(life& me, uint32_t before, uint32_t current) final;
    void on_mp(life& me, uint32_t before, uint32_t current) final;

    // listener : session
    void              on_hold(character& me) final;
    void              on_action(character& me, ACTION action, DURATION duration, uint8_t sound) final;
    void              on_updated(character& me, STATE_LEVEL level) final;
    void              on_money_changed(character& me, uint32_t value) final;
    void              on_notify(character& me, const std::string& message, MESSAGE_TYPE type) final;
    void              on_option(character& me, CUSTOM_SETTING option, bool enabled) final;
    void              on_level_up(character& me) final;
    async::task<bool> on_transfer(character& me, fb::game::map& map, const point16_t& position) final;
    void              on_item_get(character& me, const std::map<uint8_t, fb::game::item*>& items) final;
    void              on_item_changed(character& me, const std::map<uint8_t, fb::game::item*>& items) final;
    void              on_item_lost(character& me, const std::vector<uint8_t>& slots) final;

    // listener : item
    void on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr) final;
    void on_item_update(character& me, uint8_t index) final;
    void on_item_swap(character& me, uint8_t src, uint8_t dst) final;
    void on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) final;
    void on_equipment_off(character& me, EQUIPMENT_PARTS parts, uint8_t index) final;
    void on_item_active(character& me, item& item) final;
    void on_item_throws(character& me, item& item, const point16_t& to) final;

    // listener : spell
    void on_spell_update(life& me, uint8_t index) final;
    void on_spell_remove(life& me, uint8_t index) final;

    // listener : trade
    void on_trade_begin(character& me, character& you) final;
    void on_trade_bundle(character& me) final;
    void on_trade_item(character& me, character& from, uint8_t index) final;
    void on_trade_money(character& me, character& from) final;
    void on_trade_cancel(character& me, character& from) final;
    void on_trade_lock(character& me, bool mine) final;
    void on_trade_failed(character& me) final;
    void on_trade_success(character& me) final;

    // listener : dialog
    void on_dialog(character&                    me,
                   const fb::model::object&      object,
                   const std::string&            message,
                   bool                          button_prev,
                   bool                          button_next,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void on_dialog(character&                      me,
                   const fb::model::npc&           npc,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   fb::game::dialog::interaction   interaction = fb::game::dialog::interaction::NORMAL) final;
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   const std::vector<uint8_t>&   item_slots,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void on_dialog(character&                          me,
                   const fb::model::npc&               npc,
                   const std::string&                  message,
                   const fb::game::dialog::item_pairs& pairs,
                   uint16_t                            pursuit     = 0xFFFF,
                   fb::game::dialog::interaction       interaction = fb::game::dialog::interaction::NORMAL) final;
    void on_dialog(character& me, const fb::model::npc& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   const std::string&            top,
                   const std::string&            bottom,
                   int                           maxlen      = 0xFF,
                   bool                          prev        = false,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) final;

public:
    static int builtin_seed(lua_State* lua);
    static int builtin_sleep(lua_State* lua);
    static int builtin_name2mob(lua_State* lua);
    static int builtin_name2npc(lua_State* lua);
    static int builtin_name2map(lua_State* lua);
    static int builtin_name2item(lua_State* lua);
    static int builtin_pursuit_sell(lua_State* lua);
    static int builtin_pursuit_buy(lua_State* lua);
    static int builtin_sell_price(lua_State* lua);
    static int builtin_buy_price(lua_State* lua);
    static int builtin_timer(lua_State* lua);
    static int builtin_weather(lua_State* lua);
    static int builtin_name_with(lua_State* lua);
    static int builtin_assert_korean(lua_State* lua);
    static int builtin_cp949(lua_State* lua);
};

struct context::command
{
public:
    context::command_fn fn;
    bool                admin;
};

}} // namespace fb::game

template <typename T, typename... Args>
T* fb::model::object::make(fb::game::context& context, Args&&... args) const
{
    auto& model = static_cast<const typename T::model_type&>(*this);
    return context.template make<T>(model, std::forward<Args>(args)...);
}

#endif // !__FB_GAME_H__
