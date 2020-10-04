#ifndef __MASTER_H__
#define __MASTER_H__

#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <jsoncpp/json/json.h>
#include "model/map/map.h"
#include "model/item/item.h"
#include "model/npc/npc.h"
#include "model/spell/spell.h"
#include "model/mob/mob.h"
#include "model/board/board.h"
#include "model/clan/clan.h"
#include "module/encoding/encoding.h"
#include <codecvt>
#include <locale>

namespace fb { namespace game {

class session;

class master
{
private:
    typedef std::function<void(const std::string&, double)> handle_callback;
    typedef std::function<void(const std::string&, const std::string&)> handle_error;
    typedef std::function<void(uint32_t)> handle_complete;

private:
    static master*                              _instance;

public:
    std::map<uint16_t, map*>                    maps;
    std::map<uint16_t, item::master*>           items;
    std::map<uint16_t, npc::master*>            npcs;
    std::map<uint16_t, mob::master*>            mobs;
    std::map<uint16_t, spell*>                  spells;
    std::vector<class_data*>                    classes;
    std::vector<itemmix*>                       itemmixes;
    std::vector<door::master*>                  doors;
    std::vector<clan*>                          clans;
    fb::game::board                             board;

public:
    master();
    ~master();

public:
    friend class db;

private:
    bool                                        load_json(const std::string& fname, Json::Value& json);

    static fb::game::map::effects               parse_map_effect(const std::string& effect);
    static fb::game::map::options               parse_map_option(const Json::Value& data);
    static bool                                 load_map_data(uint16_t id, std::vector<char>& buffer);
    static bool                                 load_map_blocks(uint16_t id, Json::Value& buffer);

    static fb::game::item::master*              create_item(uint32_t id, const Json::Value& data);
    static fb::game::item::item_limit           parse_item_limit(const Json::Value& data);
    static fb::game::item::penalties            parse_item_penalty(const std::string& penalty);

    static fb::game::mob::sizes                 parse_mob_size(const std::string& size);
    static fb::game::mob::offensive_type        parse_mob_offensive(const std::string& offensive);

public:
    bool                                        load_maps(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_items(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_npc(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_npc_spawn(const std::string& db_fname, fb::game::listener* listener, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_mob(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_mob_spawn(const std::string& db_fname, fb::game::listener* listener, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_class(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_drop_item(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_warp(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_itemmix(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_spell(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_board(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);
    bool                                        load_door(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete);

public:
    fb::game::map*                              name2map(const std::string& name);
    fb::game::npc::master*                      name2npc(const std::string& name);
    fb::game::mob::master*                      name2mob(const std::string& name);
    fb::game::item::master*                     name2item(const std::string& name);
    fb::game::spell*                            name2spell(const std::string& name);
    const std::string*                          class2name(uint8_t cls, uint8_t promotion);
    bool                                        name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id);
    itemmix*                                    find_itemmix(const std::vector<item*>& items);
    uint32_t                                    required_exp(uint8_t class_id, uint8_t level);

public:
    static master&                              get();
    static void                                 release();
};

} }

#endif // !__MASTER_H__
