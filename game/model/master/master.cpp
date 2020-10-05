#include "model/master/master.h"
#include "model/session/session.h"

fb::game::master* fb::game::master::_instance;
fb::game::master& fb::game::master::get()
{
    if(fb::game::master::_instance == nullptr)
        fb::game::master::_instance = new master();

    return *fb::game::master::_instance;
}

void fb::game::master::release()
{
    if(fb::game::master::_instance != nullptr)
        delete fb::game::master::_instance;
}

fb::game::master::master()
{}

fb::game::master::~master()
{
    for(auto pair : this->maps)
        delete pair.second;
    this->maps.clear();

    for(auto pair : this->items)
        delete pair.second;
    this->items.clear();

    for(auto pair : this->npcs)
        delete pair.second;
    this->npcs.clear();

    for(auto pair : this->mobs)
        delete pair.second;
    this->mobs.clear();

    for(auto pair : this->spells)
        delete pair.second;
    this->spells.clear();

    for(auto cls : this->classes)
        delete cls;
    this->classes.clear();

    for(auto itemmix : this->itemmixes)
        delete itemmix;
    this->itemmixes.clear();

    for(auto door : this->doors)
        delete door;
    this->doors.clear();

    this->board.clear();
}

bool fb::game::master::load_json(const std::string& fname, Json::Value& json)
{
    auto&                   config = fb::config::get();

    std::ifstream           ifstream;
    ifstream.open(fname);
    if(ifstream.is_open() == false)
        return false;

    Json::Reader            reader;
    if(reader.parse(ifstream, json) == false)
    {
        ifstream.close();
        return false;
    }
    
    ifstream.close();
    return true;
}

fb::game::map::effects fb::game::master::parse_map_effect(const std::string& effect)
{
    if(effect == "hot wave")
        return map::effects::HOT_WAVE;

    if(effect == "shipwreck")
        return map::effects::SHIPWRECT;

    return map::effects::NO_EFFECT;
}

fb::game::map::options fb::game::master::parse_map_option(const Json::Value& data)
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

bool fb::game::master::load_map_data(uint16_t id, std::vector<char>& buffer)
{
    auto&                   config = fb::config::get();

    char                    fname[256];
    sprintf(fname, "maps/%06d.map", id);

    std::ifstream           map_file(fname, std::ios::binary);
    if(map_file.is_open() == false)
        return false;

    buffer = std::vector<char>(std::istreambuf_iterator<char>(map_file), {});
    map_file.close();

    return true;
}

bool fb::game::master::load_map_blocks(uint16_t id, Json::Value& buffer)
{
    auto&                   config = fb::config::get();

    char                    fname[256];
    sprintf(fname, "maps/%06d.block", id);
    

    std::ifstream           file(fname);
    if(file.is_open() == false)
        return false;

    Json::Reader reader;
    if(reader.parse(file, buffer) == false)
        return false;

    file.close();
    return true;
}

fb::game::item::master* fb::game::master::create_item(uint32_t id, const Json::Value& data)
{
    std::string         types           = cp949(data["type"].asString());
    std::string         name            = cp949(data["name"].asString());
    uint16_t            icon            = data["icon"].asInt() + 0xBFFF;
    uint8_t             color           = data["color"].asInt();

    if(types == "stuff")
    {
        uint16_t        capacity        = std::max(data["capacity"].asInt(), 1);
        return new fb::game::item::master(id, name, icon, color, capacity);
    }

    if(types == "consume")
    {
        std::string     bundle_type = cp949(data["bundle type"].asString());   // package Ex) 동동주
                                                                        // bundle  Ex) 도토리
        uint32_t        capacity        = data["capacity"].asInt();
        if (bundle_type == "package")
            return new fb::game::pack::master(id, name, icon, color, capacity);
        else
            return new fb::game::consume::master(id, name, icon, color, std::max(capacity, uint32_t(1)));
    }



    uint16_t look = data["equipment option"]["look"].asInt();
    if(types == "weapon")
        return new fb::game::weapon::master(id, name, icon, look, color);

    if(types == "armor")
        return new fb::game::armor::master(id, name, icon, look, color);

    if(types == "helmet")
        return new fb::game::helmet::master(id, name, icon, look, color);

    if(types == "shield")
        return new fb::game::shield::master(id, name, icon, look, color);

    if(types == "ring")
        return new fb::game::ring::master(id, name, icon, look, color);

    if(types == "auxiliary")
        return new fb::game::auxiliary::master(id, name, icon, look, color);

    if(types == "arrow")
        return new fb::game::arrow::master(id, name, icon, look, color);

    return nullptr;
}

fb::game::item::item_limit fb::game::master::parse_item_limit(const Json::Value& data)
{
    auto&                   config = fb::config::get();

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
        auto sex = cp949(data["limit"]["sex"].asString());
        if(sex == "man")
            limit.sex = fb::game::sex::MAN;
        else if(sex == "woman")
            limit.sex = fb::game::sex::WOMAN;
        else
            throw std::runtime_error(fb::game::message::assets::INVALID_SEX);
    }

    return limit;
}

fb::game::item::penalties fb::game::master::parse_item_penalty(const std::string& penalty)
{
    auto&                   config = fb::config::get();

    if(penalty.empty())
        return fb::game::item::penalties::NONE;

    if(penalty == "drop")
        return fb::game::item::penalties::DROP;


    if(penalty == "destroy")
        return fb::game::item::penalties::DESTRUCTION;

    throw std::runtime_error(fb::game::message::assets::INVALID_DEATH_PENALTY);
}

fb::game::mob::sizes fb::game::master::parse_mob_size(const std::string& size)
{
    auto&                   config = fb::config::get();

    if(size == "small")
        return mob::sizes::SMALL;

    if(size == "large")
        return mob::sizes::LARGE;

    throw std::runtime_error(fb::game::message::assets::INVALID_MOB_SIZE);
}

fb::game::mob::offensive_type fb::game::master::parse_mob_offensive(const std::string& offensive)
{
    auto&                   config = fb::config::get();

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

    throw std::runtime_error(fb::game::message::assets::INVALID_MOB_OFFENSIVE);
}

bool fb::game::master::load_maps(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             maps;
    if(this->load_json(db_fname, maps) == false)
        return false;

    auto                    count = maps.size();
    auto                    read = 0;
    for(auto i = maps.begin(); i != maps.end(); i++)
    {
        auto                data = *i;
        uint16_t            id = std::stoi(i.key().asString());
        auto                name = cp949(data["name"].asString());
        fb::game::map*      map = nullptr;

        try
        {
            // Parse common data
            uint16_t        parent  = data["parent"].asInt();
            uint8_t         bgm     = data["bgm"].asInt();
            auto            host_id = data["host"].asString();
            auto            effect  = this->parse_map_effect(cp949(data["effect"].asString()));
            auto            option  = this->parse_map_option(data);

            // Load binary
            std::vector<char> map_binary;
            if(this->load_map_data(id, map_binary) == false)
                throw std::runtime_error(fb::game::message::assets::CANNOT_LOAD_MAP_DATA);

            // Load blocks
            Json::Value     blocks;
            if(this->load_map_blocks(id, blocks) == false)
                throw std::runtime_error(fb::game::message::assets::CANNOT_LOAD_MAP_BLOCK);

            map = new fb::game::map(id, parent, host_id, bgm, name, option, effect, map_binary.data(), map_binary.size());
            for(const auto block : blocks)
                map->block(block["x"].asInt(), block["y"].asInt(), true);

            this->maps.insert(std::make_pair(id, map));

            auto            percentage = (++read * 100) / double(count);
            callback(UTF8(map->name()), percentage);
        }
        catch(std::exception& e)
        {
            if(map != nullptr)
                delete map;

            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_items(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value                     items;
    if(this->load_json(db_fname, items) == false)
        return false;

    auto                            count = items.size();
    auto                            read = 0;
    for(auto i = items.begin(); i != items.end(); i++)
    {
        fb::game::item::master*     item = nullptr;
        auto                        data = *i;
        auto                        name = cp949(data["name"].asString());

        try
        {
            // Create item core
            item = this->create_item(std::stoi(i.key().asString()), data);

            // Common options
            item->active_script(cp949(data["script"]["active"].asString()));
            item->price(data["price"].asInt());
            item->trade(data["trade"]["enabled"].asBool());
            item->entrust_enabled(data["entrust"]["enabled"].asBool());
            item->entrust_price(data["entrust"]["price"].asInt());
            item->desc(cp949(data["desc"].asString()));
            item->limit(master::parse_item_limit(data));
            item->penalty(master::parse_item_penalty(cp949(data["death penalty"].asString())));



            // Equipment options
            if(item->attr() & item::attrs::ITEM_ATTR_EQUIPMENT && data.isMember("equipment option"))
            {
                auto            option = data["equipment option"];
                auto            equipment = static_cast<equipment::master*>(item);

                equipment->durability(option["durability"].asInt());
                equipment->repair_enabled(option["repair"]["enabled"].asBool());
                equipment->repair_price((float)option["repair"]["price"].asDouble());
                equipment->rename_enabled(option["rename"]["enabled"].asBool());
                equipment->rename_price(option["rename"]["price"].asInt());
                equipment->hit(option["hit"].asInt());
                equipment->damage(option["damage"].asInt());
                equipment->strength(option["strength"].asInt());
                equipment->intelligence(option["intelligence"].asInt());
                equipment->dexteritry(option["dexteritry"].asInt());
                equipment->base_hp(option["base_hp"].asInt());
                equipment->base_mp(option["base_mp"].asInt());
                equipment->hp_percentage((float)option["hp percent"].asInt());
                equipment->mp_percentage((float)option["mp percent"].asInt());
                equipment->healing_cycle(option["healing_cycle"].asInt());
                equipment->defensive_physical(option["defensive"]["physical"].asInt());
                equipment->defensive_magical(option["defensive"]["magical"].asInt());
                equipment->dress_script(cp949(data["script"]["dress"].asString()));
                equipment->undress_script(cp949(data["script"]["undress"].asString()));
            }


            // Weapon options
            if(item->attr() & fb::game::item::attrs::ITEM_ATTR_WEAPON)
            {
                auto            option = data["equipment option"];
                auto            weapon = static_cast<fb::game::weapon::master*>(item);

                weapon->damage_small(option["damage range"]["small"]["min"].asInt(), option["damage range"]["small"]["max"].asInt());
                weapon->damage_large(option["damage range"]["large"]["min"].asInt(), option["damage range"]["large"]["max"].asInt());
                weapon->sound(option["sound"].asInt());
                weapon->spell(cp949(option["spell"].asString()));
            }


            uint32_t            id = std::stoi(i.key().asString());
            this->items.insert(std::make_pair(id, item));

            auto                percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());

            if(item != nullptr)
                delete item;
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_npc(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             npcs;
    if(this->load_json(db_fname, npcs) == false)
        return false;

    auto                    count = npcs.size();
    auto                    read = 0;
    for(auto i = npcs.begin(); i != npcs.end(); i++)
    {
        auto                data = *i;
        uint32_t            id = std::stoi(i.key().asString());
        auto                name = cp949(data["name"].asString());

        try
        {
            uint16_t        look = data["look"].asInt() + 0x7FFF;
            uint8_t         color = data["color"].asInt();

            this->npcs.insert(std::make_pair(id, new npc::master(name, look, color)));

            auto            percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_npc_spawn(const std::string& db_fname, fb::game::listener* listener, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             spawns;
    if(this->load_json(db_fname, spawns) == false)
        return false;

    auto                    count = spawns.size();
    auto                    read = 0;
    for(auto i = spawns.begin(); i != spawns.end(); i++)
    {
        auto                data = *i;
        auto                name = cp949(data["npc"].asString());
        try
        {
            auto                core = this->name2npc(name);
            if(core == nullptr)
                throw std::runtime_error(fb::game::message::assets::INVALID_NPC_NAME);


            auto                map_name = cp949(data["map"].asString());
            auto                map = this->name2map(map_name);
            if(map == nullptr)
                throw std::runtime_error(fb::game::message::assets::INVALID_MAP_NAME);

            auto                direction_str = cp949(data["direction"].asString());
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
                throw std::runtime_error(fb::game::message::assets::INVALID_NPC_DIRECTION);

            point16_t           position(data["position"]["x"].asInt(), data["position"]["y"].asInt());
            if(position.x > map->width() || position.y > map->height())
                throw std::runtime_error(fb::game::message::assets::INVALID_NPC_POSITION);
            auto                script = cp949(data["script"].asString());

            auto                cloned = new npc(core, listener);
            cloned->direction(direction);
            cloned->script(script);
            cloned->map(map, position);

            auto                percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_mob(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             mobs;
    if(this->load_json(db_fname, mobs) == false)
        return false;

    auto                    count = mobs.size();
    auto                    read = 0;
    for(auto i = mobs.begin(); i != mobs.end(); i++)
    {
        auto                data = *i;
        uint16_t            id = std::stoi(i.key().asString());
        auto                name = cp949(data["name"].asString());

        try
        {
            uint16_t        look = data["look"].asInt() + 0x7FFF;
            uint8_t         color = data["color"].asInt();
            uint32_t        base_hp = data["hp"].asInt();
            uint32_t        base_mp = data["mp"].asInt();

            auto mob = new fb::game::mob::master(name, look, color, base_hp, base_mp);
            mob->defensive_physical(data["defensive"]["physical"].asInt());
            mob->defensive_magical(data["defensive"]["magical"].asInt());
            mob->experience(data["experience"].asInt());
            mob->damage_min(data["damage"]["min"].asInt());
            mob->damage_max(data["damage"]["max"].asInt());
            mob->offensive(master::parse_mob_offensive(cp949(data["offensive"].asString())));
            mob->size(master::parse_mob_size(cp949(data["size"].asString())));
            mob->speed(data["speed"].asInt());
            mob->script_attack(cp949(data["script"]["attack"].asString()));
            mob->script_die(cp949(data["script"]["die"].asString()));

            this->mobs.insert(std::make_pair(id, mob));

            auto            percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_mob_spawn(const std::string& db_fname, fb::game::listener* listener, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             spawns;
    if(this->load_json(db_fname, spawns) == false)
        return false;

    auto                    count = spawns.size();
    auto                    read = 0;
    for(auto db_i = spawns.begin(); db_i != spawns.end(); db_i++)
    {
        auto                name = cp949(db_i.key().asString());
        auto                spawns = *db_i;

        try
        {
            auto                map = this->name2map(name);
            if(map == nullptr)
                continue;

            for(auto spawn : spawns)
            {
                auto            core = this->name2mob(cp949(spawn["name"].asString()));
                if(core == nullptr)
                    continue;

                uint16_t        x0 = spawn["area"]["x0"].asInt();
                uint16_t        x1 = spawn["area"]["x1"].asInt();
                uint16_t        y0 = spawn["area"]["y0"].asInt();
                uint16_t        y1 = spawn["area"]["y1"].asInt();
                uint16_t        count = spawn["count"].asInt();
                uint32_t        rezen = spawn["rezen time"].asInt();

                for(int i = 0; i < count; i++)
                {
                    auto        mob = static_cast<fb::game::mob*>(core->make(listener));
                    mob->spawn_point(x0, y0);
                    mob->spawn_size(x1, y1);
                    mob->respawn_time(rezen);
                    mob->map(map);
                }
            }

            auto            percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_class(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             classes;
    if(this->load_json(db_fname, classes) == false)
        return false;

    auto                    count = classes.size();
    auto                    read = 0;
    for(auto i1 = classes.begin(); i1 != classes.end(); i1++)
    {
        uint8_t             id = i1.key().asInt();      // class id
        try
        {
            auto            data = new class_data();

            auto                levels = (*i1)["levels"];
            for(auto i2 = levels.begin(); i2 != levels.end(); i2++)
            {
                uint32_t    key = i2.key().asInt();
                auto        ability = *i2;
                auto        allocated = new level_ability(ability["strength"].asInt(),
                                                          ability["intelligence"].asInt(),
                                                          ability["dexteritry"].asInt(),
                                                          ability["hp"].asInt(),
                                                          ability["mp"].asInt(),
                                                          (uint32_t)ability["exp"].asInt64());

                data->add_level_ability(allocated);
            }

            for(auto promotion: (*i1)["promotions"])
                data->add_promotion(cp949(promotion.asString()));


            this->classes.push_back(data);

            auto            percentage = (++read * 100) / double(count);
            callback(std::to_string(id), percentage);
        }
        catch(std::exception& e)
        {
            error(std::to_string(id), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_drop_item(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             drops;
    if(this->load_json(db_fname, drops) == false)
        return false;

    auto                    count = drops.size();
    auto                    read = 0;
    for(auto i1 = drops.begin(); i1 != drops.end(); i1++)
    {
        auto                name = cp949(i1.key().asString());
        auto                core = this->name2mob(name);
        try
        {
            if(core == nullptr)
                throw std::runtime_error(fb::game::message::assets::INVALID_MOB_NAME);

            auto            items = (*i1);
            for(auto i2 = items.begin(); i2 != items.end(); i2++)
            {
                float       percentage = (*i2)["percentage"].asFloat();
                auto        item_name = cp949((*i2)["item"].asString());
                auto        item_core = this->name2item(item_name);

                if(item_core == nullptr)
                    throw std::runtime_error(fb::game::message::assets::INVALID_ITEM_NAME);

                core->dropitem_add(item_core, percentage);
            }

            auto            percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_warp(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             warps;
    if(this->load_json(db_fname, warps) == false)
        return false;

    auto                    count = warps.size();
    auto                    read = 0;
    for(auto i1 = warps.begin(); i1 != warps.end(); i1++)
    {
        auto                name = cp949(i1.key().asString());

        try
        {
            auto            map = this->name2map(name);
            if(map == nullptr)
                continue;

            auto            warps = *i1;
            for(auto i2 = warps.begin(); i2 != warps.end(); i2++)
            {
                auto        next_map_name = cp949((*i2)["map"].asString());
                auto        next_map = this->name2map(next_map_name);
                if(next_map == nullptr)
                    continue;

                const point16_t before((*i2)["before"]["x"].asInt(), (*i2)["before"]["y"].asInt());
                const point16_t after((*i2)["after"]["x"].asInt(), (*i2)["after"]["y"].asInt());
                const range8_t  limit((*i2)["limit"]["min"].asInt(), (*i2)["limit"]["max"].asInt());

                map->warp_add(next_map, before, after, limit);
            }

            auto            percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_itemmix(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             itemmix_list;
    if(this->load_json(db_fname, itemmix_list) == false)
        return false;

    auto                    count = itemmix_list.size();
    auto                    read = 0;
    for(auto data : itemmix_list)
    {
        try
        {
            auto            percentage = (float)data["percentage"].asDouble();
            auto            itemmix = new fb::game::itemmix(percentage);

            for(auto require : data["require"])
            {
                auto        item = this->name2item(cp949(require["item"].asString()));
                uint32_t    count = require["count"].asInt();
                itemmix->require_add(item, count);
            }

            for(auto success: data["success"])
            {
                auto        item = this->name2item(cp949(success["item"].asString()));
                uint32_t    count = success["count"].asInt();
                itemmix->success_add(item, count);
            }

            for(auto failed: data["failed"])
            {
                auto        item = this->name2item(cp949(failed["item"].asString()));
                uint32_t    count = failed["count"].asInt();
                itemmix->failed_add(item, count);
            }

            this->itemmixes.push_back(itemmix);
        }
        catch(std::exception& e)
        {
            // error
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_spell(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             spells;
    if(this->load_json(db_fname, spells) == false)
        return false;

    auto                    count = spells.size();
    auto                    read = 0;
    for(auto i = spells.begin(); i != spells.end(); i++)
    {
        uint16_t            id = std::stoi(i.key().asString());
        const auto          data = (*i);
        const auto          name = cp949(data["name"].asString());

        try
        {
            uint8_t         type = data["type"].asInt();

            std::string         cast, uncast, concast, message;
            if (data.isMember("cast"))
                cast = cp949(data["cast"].asString());

            if (data.isMember("uncast"))
                uncast = cp949(data["uncast"].asString());

            if (data.isMember("concast"))
                concast = cp949(data["concast"].asString());

            if (data.isMember("message"))
                message = cp949(data["message"].asString());

            this->spells.insert(std::make_pair(id, new spell(spell::types(type), name, cast, uncast, concast, message)));

            auto percentage = (++read * 100) / double(count);
            callback(UTF8(name), percentage);
        }
        catch(std::exception& e)
        {
            error(UTF8(name), e.what());
        }
    }

    complete(count);
    return true;
}

bool fb::game::master::load_board(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    master::board.add("공지사항");
    auto section = this->board.add("갓승현의 역사");

    for(int i = 0; i < 100; i++)
    {
        std::stringstream sstream;
        sstream << "갓승현 연대기 - " << std::to_string(i);

        section->add(sstream.str(), sstream.str(), "갓승현");
    }

    return true;
}

bool fb::game::master::load_door(const std::string& db_fname, master::handle_callback callback, master::handle_error error, master::handle_complete complete)
{
    auto&                   config = fb::config::get();

    Json::Value             doors;
    if(this->load_json(db_fname, doors) == false)
        return false;

    for(auto& door : doors)
    {
        auto                created = new fb::game::door::master();
        for(auto& e : door)
        {
            auto            open  = e["open"].asInt();
            auto            close = e["close"].asInt();
            created->push_back(door::element(open, close));
        }

        this->doors.push_back(created);
    }

    return true;
}

fb::game::map* fb::game::master::name2map(const std::string& name)
{
    for(auto pair : this->maps)
    {
        if(pair.second->name() == name)
            return pair.second;
    }

    return nullptr;
}

fb::game::npc::master* fb::game::master::name2npc(const std::string& name)
{
    auto& db_npc = this->npcs;
    auto i = std::find_if(db_npc.begin(), db_npc.end(), 
        [&name](std::pair<uint16_t, npc::master*> pair)
        {
            return pair.second->name() == name;
        });

    return i != db_npc.end() ? (*i).second : nullptr;
}

fb::game::mob::master* fb::game::master::name2mob(const std::string& name)
{
    auto& db_mob = this->mobs;
    auto i = std::find_if(db_mob.begin(), db_mob.end(), 
        [&name](std::pair<uint16_t, mob::master*> pair)
        {
            return pair.second->name() == name;
        });

    return i != db_mob.end() ? (*i).second : nullptr;
}

fb::game::item::master* fb::game::master::name2item(const std::string& name)
{
    auto& db_item = this->items;
    auto i = std::find_if(db_item.begin(), db_item.end(), 
        [&name](std::pair<uint16_t, item::master*> pair)
        {
            return pair.second->name() == name;
        });

    return i != db_item.end() ? (*i).second : nullptr;
}

fb::game::spell* fb::game::master::name2spell(const std::string& name)
{
    auto& db_spell = this->spells;
    auto i = std::find_if(db_spell.begin(), db_spell.end(), 
        [&name](std::pair<uint16_t, spell*> pair)
        {
            return pair.second->name() == name;
        });

    return i != db_spell.end() ? (*i).second : nullptr;
}

const std::string* fb::game::master::class2name(uint8_t cls, uint8_t promotion)
{
    try
    {
        return &this->classes[cls]->promotions[promotion];
    }
    catch(std::exception&)
    {
        return nullptr;
    }
}

bool fb::game::master::name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id)
{
    for(int i1 = 0; i1 < this->classes.size(); i1++)
    {
        for(int i2 = 0; i2 < this->classes[i1]->promotions.size(); i2++)
        {
            if(this->classes[i1]->promotions[i2] != name)
                continue;

            *class_id = i1;
            *promotion_id = i2;
            return true;
        }
    }

    return false;
}

fb::game::itemmix* fb::game::master::find_itemmix(const std::vector<item*>& items)
{
    auto i = std::find_if(this->itemmixes.begin(), this->itemmixes.end(), 
        [&items](itemmix* mx)
        {
            if(mx->require.size() != items.size())
                return false;

            return mx->matched(items);
        });

    return i != this->itemmixes.end() ? *i : nullptr;
}

uint32_t fb::game::master::required_exp(uint8_t class_id, uint8_t level)
{
    try
    {
        if(class_id == 0 && level > 5)
            return 0;

        if(level > 99)
            return 0;

        return this->classes[class_id]->level_abilities[level]->exp;
    }
    catch(std::exception&)
    {
        return 0;
    }
}