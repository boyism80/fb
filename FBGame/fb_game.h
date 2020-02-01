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
#include "map.h"
#include "item.h"
#include "npc.h"
#include "mob.h"
#include "board.h"

namespace fb { namespace game {

#pragma region message
#pragma endregion

#pragma region exceptions
#pragma endregion

class acceptor : public fb_acceptor<fb::game::session>
{
private:
    enum scope { SELF, PIVOT, MAP, GLOBAL };

public:
    acceptor(uint16_t port);
    ~acceptor();

private:
    fb::ostream             make_time_stream();
    fb::ostream             make_dialog_stream(const std::string& message, bool enabled_prev, bool enabled_next);

public:
    void                    send_stream(fb::game::object& object, const fb::ostream& stream, acceptor::scope scope, bool exclude_self = false, bool encrypt = true);

public:
    bool                    handle_move_life(fb::game::life* life, fb::game::direction direction);
    void                    handle_attack_mob(fb::game::session& session, fb::game::mob& mob, uint32_t random_damage);

public:
    bool                    handle_connected(fb::game::session& session);
    bool                    handle_disconnected(fb::game::session& session);
    void                    handle_timer(uint64_t elapsed_milliseconds);

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

private:
    void                    handle_counter_mob_action(fb::game::mob* mob);
    void                    handle_containment_mob_action(fb::game::mob* mob);

public:
    void                    handle_mob_action(uint64_t now);
    void                    handle_mob_respawn(uint64_t now);
    void                    handle_session_warp(fb::game::session& session, const map::warp* warp);
};

} }

#endif // !__FB_GAME_H__
