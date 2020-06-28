#ifndef __MASTER_H__
#define __MASTER_H__

#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <json/json.h>
#include <model/map/map.h>
#include <model/item/item.h>
#include <model/npc/npc.h>
#include <model/spell/spell.h>
#include <model/mob/mob.h>
#include <model/board/board.h>
#include <model/clan/clan.h>

namespace fb { namespace game {

class session;

class master
{
private:
    static master*                              _instance;

private:
    bool                                        _initialized;

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
    static fb::game::map::effects               parse_map_effect(const std::string& effect);
    static fb::game::map::options               parse_map_option(const Json::Value& data);
    static bool                                 load_map_data(uint16_t id, std::vector<char>& buffer);
    static bool                                 load_map_blocks(uint16_t id, Json::Value& buffer);

    static fb::game::item::master*              create_item(uint32_t id, const Json::Value& data);
    static fb::game::item::item_limit           parse_item_limit(const Json::Value& data);
    static fb::game::item::penalties            parse_item_penalty(const std::string& penalty);

    static fb::game::mob::sizes                 parse_mob_size(const std::string& size);
    static fb::game::mob::offensive_type        parse_mob_offensive(const std::string& offensive);

private:
    bool                                        load_maps(const std::string& db_fname = "db/map.json");
    bool                                        load_items(const std::string& db_fname = "db/item.json");
    bool                                        load_npc(const std::string& db_fname = "db/npc.json");
    bool                                        load_npc_spawn(const std::string& db_fname = "db/npc_spawn.json", fb::game::listener* listener = nullptr);
    bool                                        load_mob(const std::string& db_fname = "db/mob.json");
    bool                                        load_mob_spawn(const std::string& db_fname = "db/mob_spawn.json", fb::game::listener* listener = nullptr);
    bool                                        load_class(const std::string& db_fname = "db/class.json");
    bool                                        load_drop_item(const std::string& db_fname = "db/item_drop.json");
    bool                                        load_warp(const std::string& db_fname = "db/warp.json");
    bool                                        load_itemmix(const std::string& db_fname = "db/itemmix.json");
    bool                                        load_spell(const std::string& db_fname = "db/spell.json");
    bool                                        load_board(const std::string& db_fname = "db/board.json");
    bool                                        load_door(const std::string& db_fname = "db/door.json");

public:
    bool                                        loads(listener*);

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
