#ifndef __FB_GAME_H__
#define __FB_GAME_H__

#include <json/json.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <module/acceptor/acceptor.h>
#include <module/config/config.h>
#include <module/timer/timer.h>
#include <model/session/session.h>
#include <model/lua/lua.h>
#include <model/master/master.h>
#include <model/group/group.h>
#include <model/clan/clan.h>
#include <model/life/life.h>
#include <model/listener/listener.h>

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

private:
    timer_container         _timer;

public:
    acceptor(boost::asio::io_context& context, uint16_t port);
    ~acceptor();

private:
    fb::ostream             make_time_stream();
    fb::ostream             make_bright_stream(uint8_t value);
    fb::game::session*      find_session(const std::string& name) const;

public:
    fb::game::session*      handle_alloc_session(fb::socket* socket);

public:
    void                    send_stream(fb::game::object& object, const fb::ostream& stream, acceptor::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send_stream(const fb::ostream& stream, const fb::game::map& map, bool encrypt = true);
    void                    send_stream(const fb::ostream& stream, bool encrypt = true);

public:
    void                    handle_level_up(fb::game::session& session);
    void                    handle_click_mob(fb::game::session& session, fb::game::mob& mob);
    void                    handle_click_npc(fb::game::session& session, fb::game::npc& npc);

    // game event method
public:
    bool                    handle_connected(fb::game::session& session);
    bool                    handle_disconnected(fb::game::session& session);
    void                    handle_timer(uint64_t elapsed_milliseconds);

    // game event method
public:
    bool                    handle_login(fb::game::session& session);
    bool                    handle_direction(fb::game::session& session);
    bool                    handle_exit(fb::game::session& session);
    bool                    handle_move(fb::game::session& session);
    bool                    handle_update_move(fb::game::session& session);
    bool                    handle_attack(fb::game::session& session);
    bool                    handle_pickup(fb::game::session& session);
    bool                    handle_emotion(fb::game::session& session);
    bool                    handle_update_map(fb::game::session& session);
    bool                    handle_refresh(fb::game::session& session);
    bool                    handle_active_item(fb::game::session& session);
    bool                    handle_inactive_item(fb::game::session& session);
    bool                    handle_drop_item(fb::game::session& session);
    bool                    handle_drop_cash(fb::game::session& session);;
    bool                    handle_front_info(fb::game::session& session);;
    bool                    handle_self_info(fb::game::session& session);;
    bool                    handle_option_changed(fb::game::session& session);
    bool                    handle_click_object(fb::game::session& session);
    bool                    handle_item_info(fb::game::session& session);
    bool                    handle_itemmix(fb::game::session& session);
    bool                    handle_trade(fb::game::session& session);
    bool                    handle_group(fb::game::session& session);
    bool                    handle_user_list(fb::game::session& session);
    bool                    handle_chat(fb::game::session& session);
    bool                    handle_board(fb::game::session& session);
    bool                    handle_swap(fb::game::session& session);
    bool                    handle_dialog(fb::game::session& session);
    bool                    handle_throw_item(fb::game::session& session);
    bool                    handle_spell(fb::game::session& session);
    bool                    handle_door(fb::game::session& session);

private:
    void                    handle_counter_mob_action(fb::game::mob* mob);
    void                    handle_containment_mob_action(fb::game::mob* mob);

public:
    void                    handle_mob_action(uint64_t now);
    void                    handle_mob_respawn(uint64_t now);
    void                    handle_buff_timer(uint64_t now);

#if defined DEBUG | defined _DEBUG
    bool                    handle_admin(fb::game::session& session, const std::string& message);
#endif

public:
    // listener : object
    void                    on_direction(fb::game::object& me);
    void                    on_show(fb::game::object& me, bool light);
    void                    on_show(fb::game::object& me, fb::game::object& you, bool light);
    void                    on_hide(fb::game::object& me);
    void                    on_hide(fb::game::object& me, fb::game::object& you);
    void                    on_move(fb::game::object& me);
    void                    on_warp(fb::game::object& me);
    void                    on_unbuff(fb::game::object& me, fb::game::buff& buff);

    // listener : life
    void                    on_attack(life& me, object* you, uint32_t damage, bool critical);
    void                    on_damage(life& me, object* you, uint32_t damage, bool critical);
    void                    on_heal_hp(life& me, uint32_t value, fb::game::object* from);
    void                    on_heal_mp(life& me, uint32_t value, fb::game::object* from);
    void                    on_die(life& me);
    void                    on_hp(life& me, uint32_t before, uint32_t current);
    void                    on_mp(life& me, uint32_t before, uint32_t current);

    // listener : session
    void                    on_attack(session& me, object* you, uint32_t damage, bool critical);
    void                    on_damage(session& me, object* you, uint32_t damage, bool critical);
    void                    on_hold(session& me);
    void                    on_die(session& me);
    void                    on_action(session& me, action action, duration duration, uint8_t sound);
    void                    on_updated(session& me, fb::game::state_level level);
    void                    on_notify(session& me, const std::string& message, message::type type);
    void                    on_option(session& me, fb::game::options option, bool enabled);
    void                    on_level_up(session& me);

    // listener : mob
    void                    on_attack(mob& me, object* you, uint32_t damage, bool critical);
    void                    on_damage(mob& me, object* you, uint32_t damage, bool critical);
    void                    on_die(mob& me);

    // listener : item
    void                    on_remove_item(session& me, uint8_t index, item::delete_attr attr);
    void                    on_update_item(session& me, uint8_t index);
    void                    on_equipment_on(session& me, item& item, equipment::slot slot);
    void                    on_equipment_off(session& me, equipment::slot slot);
    void                    on_active_item(session& me, item& item);


    // listener : trade
    void                    on_trade_begin(session& me, session& you);
    void                    on_trade_bundle(session& me);
    void                    on_trade_item(session& me, session& from, uint8_t index);
    void                    on_trade_money(session& me, session& from);
    void                    on_trade_cancel(session& me);
    void                    on_trade_lock(session& me, bool mine);
    void                    on_trade_failed(session& me);
    void                    on_trade_success(session& me);


    // built-in method
public:
    static int              builtin_name2mob(lua_State* lua);
    static int              builtin_name2npc(lua_State* lua);
    static int              builtin_name2map(lua_State* lua);
    static int              builtin_name2item(lua_State* lua);
    static int              builtin_timer(lua_State* lua);
    static int              builtin_weather(lua_State* lua);
};

} }

#endif // !__FB_GAME_H__
