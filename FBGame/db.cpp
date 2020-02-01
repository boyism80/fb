#include "db.h"

std::map<uint16_t, fb::game::map*>              fb::game::db::_maps;
std::map<uint16_t, fb::game::item::core*>       fb::game::db::_items;
std::map<uint16_t, fb::game::npc::core*>        fb::game::db::_npcs;
std::map<uint16_t, fb::game::mob::core*>        fb::game::db::_mobs;
std::map<uint16_t, fb::game::spell*>            fb::game::db::_spells;
std::vector<fb::game::class_data*>              fb::game::db::_classes;
std::vector<fb::game::itemmix*>                 fb::game::db::_itemmixes;
fb::game::board                                 fb::game::db::_board;

fb::game::db::db()
{
    this->loads();
}

fb::game::db::~db()
{
    db::release();
}

fb::game::map::effects fb::game::db::parse_map_effect(const std::string& effect)
{
    if(effect == "hot wave")
        return map::effects::HOT_WAVE;
    
    if(effect == "shipwreck")
        return map::effects::SHIPWRECT;
    
    return map::effects::NO_EFFECT;
}

fb::game::map::options fb::game::db::parse_map_option(const Json::Value& data)
{
    uint8_t                 flags = map::options::NO_OPTION;
    auto                    option = data["option"];

    if(option["build in"].asBool())
        flags |= map::options::BUILD_IN;

    if(option["enabled talk"].asBool() == false)
        flags |= map::options::DISABLE_TALK;

    if(option["enabled whisper"].asBool() == false)
        flags |= map::options::DISABLE_WHISPER;

    if(option["enabled magic"].asBool() == false)
        flags |= map::options::DISABLE_MAGIC;

    if(option["hunting ground"].asBool())
        flags |= map::options::HUNTING_GROUND;

    if(option["enabled pk"].asBool())
        flags |= map::options::ENABLE_PK;

    if(option["enabled die penalty"].asBool())
        flags |= map::options::DISABLE_DIE_PENALTY;

    return map::options(flags);
}

bool fb::game::db::load_map_data(uint16_t id, std::vector<char>& buffer)
{
    char                    fname[256];
    sprintf(fname, "map/%06d.map", id);


    std::ifstream       map_file(fname, std::ios::binary);
    if(map_file.is_open() == false)
        return false;

    buffer = std::vector<char>(std::istreambuf_iterator<char>(map_file), {});
    map_file.close();

    return true;
}

bool fb::game::db::load_map_blocks(uint16_t id, Json::Value& buffer)
{
    char                    fname[256];
    sprintf(fname, "map/%06d.block", id);

    std::ifstream           file(fname);
    if(file.is_open() == false)
        return false;

    Json::Reader reader;
    if(reader.parse(file, buffer) == false)
        return false;

    file.close();
    return true;
}

fb::game::item::core* fb::game::db::create_item(uint32_t id, const Json::Value& data)
{
    std::string         types           = data["type"].asString();
    std::string         name            = data["name"].asString();
    uint16_t            icon            = data["icon"].asInt() + 0xBFFF;
    uint8_t             color           = data["color"].asInt();

    if(types == "stuff")
    {
        uint16_t        capacity        = std::max(data["capacity"].asInt(), 1);
        return new fb::game::item::core(id, name, icon, color, capacity);
    }

    if(types == "consume")
    {
        std::string     bundle_type = data["bundle type"].asString();   // package Ex) 동동주
                                                                        // bundle  Ex) 도토리
        uint32_t        capacity        = data["capacity"].asInt();
        if (bundle_type == "package")
            return new fb::game::pack::core(id, name, icon, color, capacity);
        else
            return new fb::game::consume::core(id, name, icon, color, std::max(capacity, uint32_t(1)));
    }



    uint16_t look = data["equipment option"]["look"].asInt();
    if(types == "weapon")
        return new fb::game::weapon::core(id, name, icon, look, color);

    if(types == "armor")
        return new fb::game::armor::core(id, name, icon, look, color);

    if(types == "helmet")
        return new fb::game::helmet::core(id, name, icon, look, color);

    if(types == "shield")
        return new fb::game::shield::core(id, name, icon, look, color);

    if(types == "ring")
        return new fb::game::ring::core(id, name, icon, look, color);

    if(types == "auxiliary")
        return new fb::game::auxiliary::core(id, name, icon, look, color);

    if(types == "arrow")
        return new fb::game::arrow::core(id, name, icon, look, color);

    return nullptr;
}

fb::game::item::item_limit fb::game::db::parse_item_limit(const Json::Value& data)
{
    fb::game::item::item_limit limit;
    if(data.isMember("limit") == false)
        return limit;

    if(data["limit"].isMember("level"))
        limit.level = data["limit"]["level"].asInt();

    if(data["limit"].isMember("strength"))
        limit.strength = data["limit"]["strength"].asInt();

    if(data["limit"].isMember("dexteritry"))
        limit.dexteritry = data["limit"]["dexteritry"].asInt();

    if(data["limit"].isMember("intelligence"))
        limit.intelligence = data["limit"]["intelligence"].asInt();

    if(data["limit"].isMember("class"))
        limit.cls = data["limit"]["class"].asInt();

    if(data["limit"].isMember("promotion"))
        limit.promotion = data["limit"]["promotion"].asInt();

    if(data["limit"].isMember("sex"))
    {
        auto sex = data["limit"]["sex"].asString();
        if(sex == "man")
            limit.sex = fb::game::sex::MAN;
        else if(sex == "woman")
            limit.sex = fb::game::sex::WOMAN;
        else
            throw std::runtime_error("invalid sex limit");
    }

    return limit;
}

fb::game::item::penalties fb::game::db::parse_item_penalty(const std::string& penalty)
{
    if(penalty.empty())
        return fb::game::item::penalties::NONE;
    
    if(penalty == "drop")
        return fb::game::item::penalties::DROP;


    if(penalty == "destroy")
        return fb::game::item::penalties::DESTRUCTION;

    throw std::runtime_error("invalid death penalty");
}

fb::game::mob::sizes fb::game::db::parse_mob_size(const std::string& size)
{
    if(size == "small")
        return mob::sizes::SMALL;
    
    if(size == "large")
        return mob::sizes::LARGE;

    throw std::runtime_error("invalid mob size");
}

fb::game::mob::offensive_type fb::game::db::parse_mob_offensive(const std::string& offensive)
{
    if(offensive == "containment")
        return mob::offensive_type::CONTAINMENT;
    
    if(offensive == "counter")
        return mob::offensive_type::COUNTER;
    
    if(offensive == "none")
        return mob::offensive_type::NONE;
    
    if(offensive == "non move")
        return mob::offensive_type::NON_MOVE;
    
    if(offensive == "run away")
        return mob::offensive_type::RUN_AWAY;

    throw std::runtime_error("invalid offensive type");
}

bool fb::game::db::load_maps(const std::string& db_fname)
{
    std::ifstream           map_db;
    map_db.open(db_fname);
    if(map_db.is_open() == false)
        return false;

    Json::Value             maps;
    Json::Reader            reader;
    if(reader.parse(map_db, maps) == false)
        return false;
    map_db.close();


    for(auto i = maps.begin(); i != maps.end(); i++)
    {
        auto                data = *i;
        fb::game::map*      map = nullptr;
        
        // Parse common data
        uint16_t            id      = std::stoi(i.key().asString());
        uint16_t            parent  = data["parent"].asInt();
        uint8_t             bgm     = data["bgm"].asInt();
        auto                name    = data["name"].asString();
        auto                effect  = db::parse_map_effect(data["effect"].asString());
        auto                option  = db::parse_map_option(data);

        try
        {
            // Load binary
            std::vector<char> map_binary;
            if(db::load_map_data(id, map_binary) == false)
                throw std::runtime_error("cannot load map data");

            // Load blocks
            Json::Value     blocks;
            if(db::load_map_blocks(id, blocks) == false)
                throw std::runtime_error("cannot load map blocks");

            map = new fb::game::map(id, parent, bgm, name, option, effect, map_binary.data(), map_binary.size());
            for(const auto block : blocks)
                map->block(block["x"].asInt(), block["y"].asInt(), true);
            
            db::_maps.insert(std::make_pair(id, map));
        }
        catch(std::exception& e)
        {
            if(map != nullptr)
                delete map;

            std::cout << name << " : " << e.what() << std::endl;
        }
    }

    return true;
}

bool fb::game::db::load_items(const std::string& db_fname)
{
    // Load item db
    std::ifstream           item_db;
    item_db.open(db_fname);
    if(item_db.is_open() == false)
        return false;

    // Parse json
    Json::Value             items;
    Json::Reader            reader;
    if(reader.parse(item_db, items) == false)
        return false;
    item_db.close();

    for(auto i = items.begin(); i != items.end(); i++)
    {
        fb::game::item::core*   item = nullptr;

        try
        {
            auto                data = *i;
            
            
            // Create item core
            item = db::create_item(i.index(), data);
            
            // Common options
            item->active_script(data["script"]["active"].asString());
            item->price(data["price"].asInt());
            item->trade(data["trade"]["enabled"].asBool());
            item->entrust_enabled(data["entrust"]["enabled"].asBool());
            item->entrust_price(data["entrust"]["price"].asInt());
            item->desc(data["desc"].asString());
            item->limit(db::parse_item_limit(data));
            item->penalty(db::parse_item_penalty(data["death penalty"].asString()));



            // Equipment options
            if(item->attr() & item::attrs::ITEM_ATTR_EQUIPMENT && data.isMember("equipment option"))
            {
                auto            option = data["equipment option"];
                auto            equipment = static_cast<equipment::core*>(item);

                equipment->durability(option["durability"].asInt());
                equipment->repair_enabled(option["repair"]["enabled"].asBool());
                equipment->repair_price(option["repair"]["price"].asDouble());
                equipment->rename_enabled(option["rename"]["enabled"].asBool());
                equipment->rename_price(option["rename"]["price"].asInt());
                equipment->hit(option["hit"].asInt());
                equipment->damage(option["damage"].asInt());
                equipment->strength(option["strength"].asInt());
                equipment->intelligence(option["intelligence"].asInt());
                equipment->dexteritry(option["dexteritry"].asInt());
                equipment->base_hp(option["base_hp"].asInt());
                equipment->base_mp(option["base_mp"].asInt());
                equipment->hp_percentage(option["hp percent"].asInt());
                equipment->mp_percentage(option["mp percent"].asInt());
                equipment->healing_cycle(option["healing_cycle"].asInt());
                equipment->defensive_physical(option["defensive"]["physical"].asInt());
                equipment->defensive_magical(option["defensive"]["magical"].asInt());
                equipment->dress_script(data["script"]["dress"].asString());
                equipment->undress_script(data["script"]["undress"].asString());
            }


            // Weapon options
            if(item->attr() & item::attrs::ITEM_ATTR_WEAPON)
            {
                auto            option = data["equipment option"];
                auto            weapon = static_cast<fb::game::weapon::core*>(item);

                weapon->damage_small(option["damage range"]["small"]["min"].asInt(), option["damage range"]["small"]["max"].asInt());
                weapon->damage_large(option["damage range"]["large"]["min"].asInt(), option["damage range"]["large"]["max"].asInt());
                weapon->sound(option["sound"].asInt());
                weapon->spell(option["spell"].asString());
            }


            uint32_t            id = std::stoi(i.key().asString());
            db::_items.insert(std::make_pair(id, item));
        }
        catch(std::exception& e)
        {
            if(item != nullptr)
            {
                std::cout << item->name() << " : " << e.what();
                delete item;
            }
            else
            {
                std::cout << "What the fuck? : " << e.what();
            }
        }
    }

    return true;
}

bool fb::game::db::load_npc(const std::string& db_fname)
{
    std::ifstream           npc_db;
    npc_db.open(db_fname);
    if(npc_db.is_open() == false)
        return false;

    Json::Value             npcs;
    Json::Reader            reader;
    if(reader.parse(npc_db, npcs) == false)
        return false;
    npc_db.close();

    for(auto i = npcs.begin(); i != npcs.end(); i++)
    {
        uint32_t            id = std::stoi(i.key().asString());
        Json::Value         json = *i;

        std::string         name = json["name"].asString();
        uint16_t            look = json["look"].asInt() + 0x7FFF;
        uint8_t             color = json["color"].asInt();

        db::_npcs.insert(std::make_pair(id, new npc::core(id, name, look, color)));
    }

    return true;
}

bool fb::game::db::load_npc_spawn(const std::string& db_fname)
{
    std::ifstream           db_spawns;
    db_spawns.open(db_fname);
    if(db_spawns.is_open() == false)
        return false;

    Json::Value             spawns;
    Json::Reader            reader;
    if(reader.parse(db_spawns, spawns) == false)
        return false;
    db_spawns.close();


    for(auto i = spawns.begin(); i != spawns.end(); i++)
    {
        auto                data = *i;
        auto                npc_name = data["npc"].asString();
        auto                core = db::name2npc(npc_name);
        if(core == NULL)
        {
            std::cout << "존재하지 않는 NPC입니다. : " << npc_name << std::endl;
            continue;
        }


        auto                map_name = data["map"].asString();
        auto                map = db::name2map(map_name);
        if(map == NULL)
        {
            std::cout << "존재하지 않는 맵입니다. : " << map_name << std::endl;
            continue;
        }

        auto                direction_str = data["direction"].asString();
        auto                direction = fb::game::direction::BOTTOM;
        if(direction_str == "top")
            direction = fb::game::direction::TOP;
        else if(direction_str == "right")
            direction = fb::game::direction::RIGHT;
        else if(direction_str == "bottom")
            direction = fb::game::direction::BOTTOM;
        else if(direction_str == "left")
            direction = fb::game::direction::LEFT;
        else
        {
            std::cout << "NPC의 방향이 올바르지 않습니다. : " << npc_name << std::endl;
            continue;
        }

        point16_t           position(data["position"]["x"].asInt(), data["position"]["y"].asInt());
        auto                script = data["script"].asString();

        auto                cloned = new npc(core);
        cloned->direction(direction);
        cloned->map(map, position);
        cloned->script(script);
    }
    return true;
}

bool fb::game::db::load_mob(const std::string& db_fname)
{
    std::ifstream           db_mob;
    db_mob.open(db_fname);
    if(db_mob.is_open() == false)
        return false;

    Json::Value             mobs;
    Json::Reader            reader;
    if(reader.parse(db_mob, mobs) == false)
        return false;
    db_mob.close();


    for(auto i = mobs.begin(); i != mobs.end(); i++)
    {
        uint16_t            id = std::stoi(i.key().asString());
        auto                data = *i;
        
        auto                name = data["name"].asString();
        uint16_t            look = data["look"].asInt() + 0x7FFF;
        uint8_t             color = data["color"].asInt();
        uint32_t            base_hp = data["hp"].asInt();
        uint32_t            base_mp = data["mp"].asInt();

        auto mob = new fb::game::mob::core(id, name, look, color, base_hp, base_mp);
        mob->defensive_physical(data["defensive"]["physical"].asInt());
        mob->defensive_magical(data["defensive"]["magical"].asInt());
        mob->experience(data["experience"].asInt());
        mob->damage_min(data["damage"]["min"].asInt());
        mob->damage_max(data["damage"]["max"].asInt());
        mob->offensive(db::parse_mob_offensive(data["offensive"].asString()));
        mob->size(db::parse_mob_size(data["size"].asString()));
        mob->speed(data["speed"].asInt());
        mob->script_attack(data["script"]["attack"].asString());
        mob->script_die(data["script"]["die"].asString());

        db::_mobs.insert(std::make_pair(id, mob));
    }

    return true;
}

bool fb::game::db::load_mob_spawn(const std::string& db_fname)
{
    std::ifstream           db_spawns;
    db_spawns.open(db_fname);
    if(db_spawns.is_open() == false)
        return false;

    Json::Value             spawns;
    Json::Reader            reader;
    if(reader.parse(db_spawns, spawns) == false)
        return false;
    db_spawns.close();


    for(auto db_i = spawns.begin(); db_i != spawns.end(); db_i++)
    {
        auto                map_name = db_i.key().asString();
        auto                spawns = *db_i;

        fb::game::map* map = db::name2map(map_name);
        if(map == NULL)
            continue;

        for(auto spawn : spawns)
        {
            auto            core = db::name2mob(spawn["name"].asString());
            if(core == NULL)
                continue;

            uint16_t        x0 = spawn["area"]["x0"].asInt();
            uint16_t        x1 = spawn["area"]["x1"].asInt();
            uint16_t        y0 = spawn["area"]["y0"].asInt();
            uint16_t        y1 = spawn["area"]["y1"].asInt();
            uint16_t        count = spawn["count"].asInt();
            uint32_t        rezen = spawn["rezen time"].asInt();

            for(int i = 0; i < count; i++)
            {
                auto        mob = static_cast<fb::game::mob*>(core->make());
                mob->spawn_point(x0, y0);
                mob->spawn_size(x1, y1);
                mob->respawn_time(rezen);
                mob->map(map);
            }
        }
    }
    return true;
}

bool fb::game::db::load_class(const std::string& db_fname)
{
    std::ifstream           db_classes;
    db_classes.open(db_fname);
    if(db_classes.is_open() == false)
        return false;

    Json::Value             classes;
    Json::Reader            reader;
    if(reader.parse(db_classes, classes) == false)
        return false;
    db_classes.close();

    for(auto i1 = classes.begin(); i1 != classes.end(); i1++)
    {
        uint8_t             class_id = i1.key().asInt();
        auto                cdata = new class_data();

        auto                levels = (*i1)["levels"];
        for(auto i2 = levels.begin(); i2 != levels.end(); i2++)
        {
            uint32_t        key = i2.key().asInt();
            auto            ability = *i2;
            auto            allocated = new level_ability(ability["strength"].asInt(),
                                                          ability["intelligence"].asInt(),
                                                          ability["dexteritry"].asInt(),
                                                          ability["hp"].asInt(),
                                                          ability["mp"].asInt(),
                                                          ability["exp"].asInt64());

            cdata->add_level_ability(allocated);
        }

        for(auto promotion: (*i1)["promotions"])
            cdata->add_promotion(promotion.asString());


        db::_classes.push_back(cdata);
    }

    return true;
}

bool fb::game::db::load_drop_item(const std::string& db_fname)
{
    std::ifstream           db_drop(db_fname);
    if(db_drop.is_open() == false)
        return false;

    Json::Value             drops;
    Json::Reader            reader;
    if(reader.parse(db_drop, drops) == false)
        return false;

    for(auto i1 = drops.begin(); i1 != drops.end(); i1++)
    {
        auto                mob_name = i1.key().asString();
        auto                mob_core = db::name2mob(mob_name);
        try
        {
            if(mob_core == nullptr)
                throw std::runtime_error("invalid mob name");

            auto            items = (*i1);
            for(auto i2 = items.begin(); i2 != items.end(); i2++)
            {
                float       percentage = (*i2)["percentage"].asFloat();
                auto        item_name = (*i2)["item"].asString();
                auto        item_core = db::name2item(item_name);

                if(item_core == NULL)
                    throw std::runtime_error("invalid item name");

                mob_core->dropitem_add(item_core, percentage);
            }
        }
        catch(std::exception& e)
        {
            std::cout << e.what() << " : " << mob_name;
        }
    }

    db_drop.close();
    return true;
}

bool fb::game::db::load_warp(const std::string& db_fname)
{
    std::ifstream           db_warp(db_fname);
    if(db_warp.is_open() == false)
        return false;

    Json::Value             warps;
    Json::Reader            reader;
    if(reader.parse(db_warp, warps) == false)
        return false;

    for(auto i1 = warps.begin(); i1 != warps.end(); i1++)
    {
        auto                map_name = i1.key().asString();
        auto                map = db::name2map(map_name);
        if(map == NULL)
            continue;

        auto                warps = *i1;
        for(auto i2 = warps.begin(); i2 != warps.end(); i2++)
        {
            auto            next_map_name = (*i2)["map"].asString();
            auto            next_map = db::name2map(next_map_name);
            if(next_map == NULL)
                continue;

            const point16_t before((*i2)["before"]["x"].asInt(), (*i2)["before"]["y"].asInt());
            const point16_t after((*i2)["after"]["x"].asInt(), (*i2)["after"]["y"].asInt());
            const range8_t  limit((*i2)["limit"]["min"].asInt(), (*i2)["limit"]["max"].asInt());

            map->warp_add(next_map, before, after, limit);
        }
    }

    db_warp.close();
    return true;
}

bool fb::game::db::load_itemmix(const std::string& db_fname)
{
    std::ifstream           db_mix(db_fname);
    if(db_mix.is_open() == false)
        return false;

    Json::Value             itemmix_list;
    Json::Reader            reader;
    if(reader.parse(db_mix, itemmix_list) == false)
        return false;

    for(auto json : itemmix_list)
    {
        float               percentage = json["percentage"].asDouble();
        auto                itemmix = new fb::game::itemmix(percentage);

        for(auto require : json["require"])
        {
            auto            item = db::name2item(require["item"].asString());
            uint32_t        count = require["count"].asInt();
            itemmix->require_add(item, count);
        }

        for(auto success: json["success"])
        {
            auto            item = db::name2item(success["item"].asString());
            uint32_t        count = success["count"].asInt();
            itemmix->success_add(item, count);
        }

        for(auto failed: json["failed"])
        {
            auto            item = db::name2item(failed["item"].asString());
            uint32_t        count = failed["count"].asInt();
            itemmix->failed_add(item, count);
        }

        db::_itemmixes.push_back(itemmix);
    }

    db_mix.close();
    return true;
}

bool fb::game::db::load_spell(const std::string& db_fname)
{
    std::ifstream           db_spell(db_fname);
    if(db_spell.is_open() == false)
        return false;

    Json::Value             spells;
    Json::Reader            reader;
    if(reader.parse(db_spell, spells) == false)
        return false;

    for(auto i = spells.begin(); i != spells.end(); i++)
    {
        uint16_t            id = std::stoi(i.key().asString());
        const auto          data = (*i);

        const auto          name = data["name"].asString();
        uint8_t             type = data["type"].asInt();

        std::string         cast, uncast, concast, message;
        if (data.isMember("cast"))
            cast = data["cast"].asString();

        if (data.isMember("uncast"))
            uncast = data["uncast"].asString();

        if (data.isMember("concast"))
            concast = data["concast"].asString();

        if (data.isMember("message"))
            message = data["message"].asString();

        db::_spells.insert(std::make_pair(id, new spell(type, name, cast, uncast, concast, message)));
    }

    db_spell.close();
    return true;
}

bool fb::game::db::load_board(const std::string& db_fname)
{
    db::_board.add("공지사항");
    auto section = db::_board.add("갓승현의 역사");

    for(int i = 0; i < 100; i++)
    {
        std::stringstream sstream;
        sstream << "갓승현 연대기 - " << std::to_string(i);

        section->add(i+1, sstream.str(), sstream.str(), "갓승현");
    }

    return true;
}

bool fb::game::db::loads()
{
    db::release();

    if(db::load_spell("db/spell.json") == false)
        return false;
    
    if(db::load_maps("db/map.json") == false)
        return false;
    
    if(db::load_warp("db/warp.json") == false)
        return false;
    
    if(db::load_items("db/item.json") == false)
        return false;
    
    if(db::load_itemmix("db/itemmix.json") == false)
        return false;
    
    if(db::load_npc("db/npc.json") == false)
        return false;
    
    if(db::load_mob("db/mob.json") == false)
        return false;
    
    if(db::load_drop_item("db/item_drop.json") == false)
        return false;
    
    if(db::load_npc_spawn("db/npc_spawn.json") == false)
        return false;
    
    if(db::load_mob_spawn("db/mob_spawn.json") == false)
        return false;
    
    if(db::load_class() == false)
        return false;
    
    if(db::load_board("db/board.json") == false)
        return false;
    
    return true;
}

void fb::game::db::release()
{
    for(auto pair : db::_maps)
        delete pair.second;
    db::_maps.clear();

    for(auto pair : db::_items)
        delete pair.second;
    db::_items.clear();

    for(auto pair : db::_npcs)
        delete pair.second;
    db::_npcs.clear();

    for(auto pair : db::_mobs)
        delete pair.second;
    db::_mobs.clear();

    for(auto pair : db::_spells)
        delete pair.second;
    db::_spells.clear();

    for(auto cls : db::_classes)
        delete cls;
    db::_classes.clear();

    for(auto itemmix : db::_itemmixes)
        delete itemmix;
    db::_itemmixes.clear();
}

std::map<uint16_t, fb::game::map*>& fb::game::db::maps()
{
    return db::_maps;
}

std::map<uint16_t, fb::game::item::core*>& fb::game::db::items()
{
    return db::_items;
}

std::map<uint16_t, fb::game::npc::core*>& fb::game::db::npcs()
{
    return db::_npcs;
}

std::map<uint16_t, fb::game::mob::core*>& fb::game::db::mobs()
{
    return db::_mobs;
}

std::map<uint16_t, fb::game::spell*>& fb::game::db::spells()
{
    return db::_spells;
}

std::vector<fb::game::class_data*>& fb::game::db::classes()
{
    return db::_classes;
}

std::vector<fb::game::itemmix*>& fb::game::db::itemmixes()
{
    return db::_itemmixes;
}

fb::game::board& fb::game::db::board()
{
    return db::_board;
}

fb::game::map* fb::game::db::name2map(const std::string& name)
{
    for(auto pair : db::_maps)
    {
        if(pair.second->name() == name)
            return pair.second;
    }

    return NULL;
}

fb::game::npc::core* fb::game::db::name2npc(const std::string& name)
{
    for(auto pair : db::_npcs)
    {
        if(pair.second->name() == name)
            return pair.second;
    }

    return NULL;
}

fb::game::mob::core* fb::game::db::name2mob(const std::string& name)
{
    for(auto pair : db::_mobs)
    {
        if (pair.second->name() == name)
            return pair.second;
    }

    return NULL;
}

fb::game::item::core* fb::game::db::name2item(const std::string& name)
{
    for(auto item : db::_items)
    {
        if(item.second->name() == name)
            return item.second;
    }

    return NULL;
}

const std::string* fb::game::db::class2name(uint8_t cls, uint8_t promotion)
{
    try
    {
        return &db::_classes[cls]->promotions[promotion];
    }
    catch(std::exception& e)
    {
        return NULL;
    }
}

bool fb::game::db::name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id)
{
    for(int i1 = 0; i1 < db::_classes.size(); i1++)
    {
        for(int i2 = 0; i2 < db::_classes[i1]->promotions.size(); i2++)
        {
            if(db::_classes[i1]->promotions[i2] != name)
                continue;

            *class_id = i1;
            *promotion_id = i2;
            return true;
        }
    }

    return false;
}

fb::game::itemmix* fb::game::db::find_itemmix(const std::vector<item*>& items)
{
    for(const auto itemmix : db::_itemmixes)
    {
        if(itemmix->require.size() != items.size())
            continue;

        if(itemmix->matched(items))
            return itemmix;
    }

    return NULL;
}

uint32_t fb::game::db::required_exp(uint8_t class_id, uint8_t level)
{
    try
    {
        return db::_classes[class_id]->level_abilities[level]->exp;
    }
    catch(std::exception& e)
    {
        return 0;
    }
}