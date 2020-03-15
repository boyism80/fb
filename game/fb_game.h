#ifndef __FB_GAME_H__
#define __FB_GAME_H__

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include "json/json.h"
#include "acceptor.h"
#include "session.h"
#include "lua.h"
#include "db.h"

namespace fb { namespace game {

#pragma region message
#pragma endregion

#pragma region exceptions
#pragma endregion

class acceptor : public fb_acceptor<fb::game::session>
{
public:
    LUA_PROTOTYPE

public:
    enum scope { SELF, PIVOT, MAP, WORLD };

public:
    acceptor(uint16_t port);
    ~acceptor();

private:
    fb::ostream             make_time_stream();
    fb::ostream             make_bright_stream(uint8_t value);

public:
    fb::game::session*      handle_allocate_session(SOCKET fd);

public:
    void                    send_stream(fb::game::object& object, const fb::ostream& stream, acceptor::scope scope, bool exclude_self = false, bool encrypt = true);
    void                    send_stream(const fb::ostream& stream, bool encrypt = true);

public:
    bool                    handle_move_life(fb::game::life* life, fb::game::direction direction);
    void                    handle_damage(fb::game::session& session, fb::game::mob& mob, uint32_t damage);
    void                    handle_damage(fb::game::life& life, uint32_t damage);
    void                    handle_damage(fb::game::session& session, uint32_t damage);
    void                    handle_die(fb::game::mob& mob);
    void                    handle_die(fb::game::session& session);
    void                    handle_experience(fb::game::session& session, uint32_t exp, bool limit = true);
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

private:
    void                    handle_counter_mob_action(fb::game::mob* mob);
    void                    handle_containment_mob_action(fb::game::mob* mob);

public:
    void                    handle_mob_action(uint64_t now);
    void                    handle_mob_respawn(uint64_t now);
    void                    handle_session_warp(fb::game::session& session, const map::warp* warp);

#if defined DEBUG | defined _DEBUG
    bool                    handle_admin(fb::game::session& session, const std::string& message);
#endif

    // built-in method
public:
    static int              builtin_name2mob(lua_State* lua);
    static int              builtin_name2npc(lua_State* lua);
    static int              builtin_name2item(lua_State* lua);
    static int              builtin_timer(lua_State* lua);
    static int              builtin_weather(lua_State* lua);
};

} }

#endif // !__FB_GAME_H__
