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

private:
    static acceptor*                    _instance;

private:
    std::map<uint16_t, map*>            _maps;
    std::map<uint16_t, item::core*>     _items;
    std::map<uint16_t, npc::core*>      _npcs;
    std::map<uint16_t, mob::core*>      _mobs;
    std::vector<class_data*>            _classes;
    std::vector<itemmix*>               _itemmixes;
    board                               _board;

public:
    acceptor(uint16_t port);
    ~acceptor();

private:
    bool                    convert_map_file(const std::string& db_fname = "db/map_db.txt");
    bool                    convert_block_file(const map* map, const char* ext = "block");
    bool                    convert_item_file(const std::string& db_fname = "db/item_db.txt");
    bool                    convert_npc_file(const std::string& db_fname = "db/npc_db.txt");
    bool                    convert_npc_spawn_file(const std::string& db_fname = "db/npcspawn.txt");
    bool                    convert_mob_file(const std::string& db_fname = "db/mob_db.txt");
    bool                    convert_mob_spawn_file(const std::string& db_fname = "db/mobspawn.txt");
    bool                    convert_class_file(const std::string& db_fname = "db/level_db.txt");
    bool                    load_maps(const std::string& db_fname = "db/map.json");
    bool                    load_items(const std::string& db_fname = "db/item.json");
    bool                    load_npc(const std::string& db_fname = "db/npc.json");
    bool                    load_npc_spawn(const std::string& db_fname = "db/npc_spawn.json");
    bool                    load_mob(const std::string& db_fname = "db/mob.json");
    bool                    load_mob_spawn(const std::string& db_fname = "db/mob_spawn.json");
    bool                    load_class(const std::string& db_fname = "db/class.json");
    bool                    load_drop_item(const std::string& db_fname = "db/item_drop.json");
    bool                    load_warp(const std::string& db_fname = "db/warp.json");
    bool                    load_itemmix(const std::string& db_fname = "db/itemmix.json");

public:
    fb::game::map*          name2map(const std::string& name) const;
    fb::game::npc::core*    name2npc(const std::string& name) const;
    fb::game::mob::core*    name2mob(const std::string& name) const;
    fb::game::item::core*   name2item(const std::string& name) const;
    const std::string*      class2name(uint8_t cls, uint8_t promotion) const;
    bool                    name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id) const;
    itemmix*                find_itemmix(const std::vector<item*>& items);

    uint32_t                required_exp(uint8_t class_id, uint8_t level);

private:
    fb::ostream             make_time_stream();
    fb::ostream             make_message_stream(const std::string& message, message::type types);
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

private:
    void                    handle_counter_mob_action(fb::game::mob* mob);
    void                    handle_containment_mob_action(fb::game::mob* mob);

public:
    void                    handle_mob_action(uint64_t now);
    void                    handle_mob_respawn(uint64_t now);
    void                    handle_session_warp(fb::game::session& session, const map::warp* warp);

public:
    static acceptor*        instance();
    static void             release();
};

} }

#endif // !__FB_GAME_H__
