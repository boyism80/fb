#ifndef __DB_H__
#define __DB_H__

#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include "json/json.h"
#include "map.h"
#include "item.h"
#include "npc.h"
#include "spell.h"
#include "mob.h"
#include "board.h"

namespace fb { namespace game {

static class db
{
private:
    static std::map<uint16_t, map*>         _maps;
    static std::map<uint16_t, item::core*>  _items;
    static std::map<uint16_t, npc::core*>   _npcs;
    static std::map<uint16_t, mob::core*>   _mobs;
    static std::map<uint16_t, spell*>       _spells;
    static std::vector<class_data*>         _classes;
    static std::vector<itemmix*>            _itemmixes;
    static fb::game::board                  _board;

private:
    db();
    ~db();

private:
    static fb::game::map::effects           parse_map_effect(const std::string& effect);
    static fb::game::map::options           parse_map_option(const Json::Value& data);
    static bool                             load_map_data(uint16_t id, std::vector<char>& buffer);
    static bool                             load_map_blocks(uint16_t id, Json::Value& buffer);

    static fb::game::item::core*            create_item(uint32_t id, const Json::Value& data);
    static fb::game::item::item_limit       parse_item_limit(const Json::Value& data);
    static fb::game::item::penalties        parse_item_penalty(const std::string& penalty);

    static fb::game::mob::sizes             parse_mob_size(const std::string& size);
    static fb::game::mob::offensive_type    parse_mob_offensive(const std::string& offensive);

private:
    static bool                             load_maps(const std::string& db_fname = "db/map.json");
    static bool                             load_items(const std::string& db_fname = "db/item.json");
    static bool                             load_npc(const std::string& db_fname = "db/npc.json");
    static bool                             load_npc_spawn(const std::string& db_fname = "db/npc_spawn.json");
    static bool                             load_mob(const std::string& db_fname = "db/mob.json");
    static bool                             load_mob_spawn(const std::string& db_fname = "db/mob_spawn.json");
    static bool                             load_class(const std::string& db_fname = "db/class.json");
    static bool                             load_drop_item(const std::string& db_fname = "db/item_drop.json");
    static bool                             load_warp(const std::string& db_fname = "db/warp.json");
    static bool                             load_itemmix(const std::string& db_fname = "db/itemmix.json");
    static bool                             load_spell(const std::string& db_fname = "db/spell.json");
    static bool                             load_board(const std::string& db_fname = "db/board.json");

public:
    static bool                             loads();
    static void                             release();

public:
    static std::map<uint16_t, map*>&        maps();
    static std::map<uint16_t, item::core*>& items();
    static std::map<uint16_t, npc::core*>&  npcs();
    static std::map<uint16_t, mob::core*>&  mobs();
    static std::map<uint16_t, spell*>&      spells();
    static std::vector<class_data*>&        classes();
    static std::vector<itemmix*>&           itemmixes();
    static fb::game::board&                 board();

    static fb::game::map*                   name2map(const std::string& name);
    static fb::game::npc::core*             name2npc(const std::string& name);
    static fb::game::mob::core*             name2mob(const std::string& name);
    static fb::game::item::core*            name2item(const std::string& name);
    static const std::string*               class2name(uint8_t cls, uint8_t promotion);
    static bool                             name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id);
    static itemmix*                         find_itemmix(const std::vector<item*>& items);
    static uint32_t                         required_exp(uint8_t class_id, uint8_t level);
};

} }


#endif // !__DB_H__
