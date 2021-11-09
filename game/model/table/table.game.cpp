#include "model/session/session.h"
#include "table.game.h"
#include <regex>

void assert_script(const std::string& script, const std::string& regex, const std::string& fnName, const std::function<void(const std::string&)>& callback)
{
    if(script.empty())
        return;

    if(std::filesystem::exists(script) == false)
    {
        std::stringstream sstream;
        sstream << script
                << " 파일이 존재하지 않습니다.";

        callback(script);
        throw std::runtime_error(sstream.str());
    }

    auto in = std::ifstream(script);
    auto buffer = std::stringstream();
    buffer << in.rdbuf();


    auto re = std::regex(regex);
    auto matches = std::smatch();
    auto source = buffer.str();
    if(std::regex_search(source, matches, re) == false)
    {
        std::stringstream sstream;
        sstream << script
                << " 파일에 "
                << fnName
                << " 함수가 존재하지 않습니다.";

        callback(script);
        throw std::runtime_error(sstream.str());
    }
}

std::vector<fb::game::clan*> fb::game::table::clans;
fb::game::board              fb::game::table::board;
fb::game::container::map     fb::game::table::maps;
fb::game::container::worlds  fb::game::table::worlds;
fb::game::container::item    fb::game::table::items;
fb::game::container::npc     fb::game::table::npcs;
fb::game::container::mob     fb::game::table::mobs;
fb::game::container::spell   fb::game::table::spells;
fb::game::container::cls     fb::game::table::classes;
fb::game::container::mix     fb::game::table::mixes;
fb::game::container::door    fb::game::table::doors;

fb::game::table::table()
{ }

fb::game::table::~table()
{ }

fb::game::map* fb::game::container::map::name2map(const std::string& name)
{
    for(auto& pair : *this)
    {
        if(pair.second->name() == name)
            return pair.second;
    }

    return nullptr;
}

void fb::game::container::map::clear()
{
    for(auto& x : *this)
        delete x.second;

    std::map<uint16_t, fb::game::map*>::clear();
}

void fb::game::container::map::clear(const std::function<bool(const fb::game::map&)>& fn)
{
    for(auto i = this->begin(); i != this->end();)
    {
        if(fn(*i->second))
        {
            delete i->second;
            this->erase(i++);
        }
        else
        {
            ++i;
        }
    }
}

fb::game::map* fb::game::container::map::operator[](uint16_t id) const
{
    auto found = this->find(id);
    if(found == this->end())
        return nullptr;

    return found->second;
}

fb::game::npc::master* fb::game::container::npc::name2npc(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](std::pair<uint16_t, fb::game::npc::master*> pair)
        {
            return pair.second->name == name;
        });

    return i != this->end() ? (*i).second : nullptr;
}

fb::game::mob::master* fb::game::container::mob::name2mob(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](std::pair<uint16_t, fb::game::mob::master*> pair)
        {
            return pair.second->name == name;
        });

    return i != this->end() ? (*i).second : nullptr;
}

fb::game::item::master* fb::game::container::item::name2item(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](std::pair<uint16_t, fb::game::item::master*> pair)
        {
            return pair.second->name == name;
        });

    return i != this->end() ? (*i).second : nullptr;
}

fb::game::spell* fb::game::container::spell::name2spell(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](std::pair<uint16_t, fb::game::spell*> pair)
        {
            return pair.second->name() == name;
        });

    return i != this->end() ? (*i).second : nullptr;
}

const std::string* fb::game::container::cls::class2name(uint8_t cls, uint8_t promotion)
{
    try
    {
        return &this->operator[](cls)->promotions[promotion];
    }
    catch(std::exception&)
    {
        return nullptr;
    }
}

bool fb::game::container::cls::name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id)
{
    for(int i1 = 0; i1 < this->size(); i1++)
    {
        for(int i2 = 0; i2 < this->operator[](i1)->promotions.size(); i2++)
        {
            if(this->operator[](i1)->promotions[i2] != name)
                continue;

            *class_id = i1;
            *promotion_id = i2;
            return true;
        }
    }

    return false;
}

uint32_t fb::game::container::cls::exp(uint8_t class_id, uint8_t level)
{
    try
    {
        if(class_id == 0 && level >= 5)
            return 0;

        if(level > 99)
            return 0;

        return this->operator[](class_id)->abilities[level]->exp;
    }
    catch(std::exception&)
    {
        return 0;
    }
}

fb::game::map::effects fb::game::container::map::to_effect(const std::string& effect)
{
    if(effect == "fire")
        return fb::game::map::effects::FIRE;

    if(effect == "water")
        return fb::game::map::effects::WATER;

    return fb::game::map::effects::NONE;
}

fb::game::map::options fb::game::container::map::to_option(const Json::Value& data)
{
    auto                    flags = fb::game::map::options::NO_OPTION;
    auto&                   option = data["option"];

    if(option["build in"].asBool())
        flags |= fb::game::map::options::BUILD_IN;

    if(option["enabled talk"].asBool() == false)
        flags |= fb::game::map::options::DISABLE_TALK;

    if(option["enabled whisper"].asBool() == false)
        flags |= fb::game::map::options::DISABLE_WHISPER;

    if(option["enabled magic"].asBool() == false)
        flags |= fb::game::map::options::DISABLE_MAGIC;

    if(option["hunting ground"].asBool())
        flags |= fb::game::map::options::HUNTING_GROUND;

    if(option["enabled pk"].asBool())
        flags |= fb::game::map::options::ENABLE_PK;

    if(option["enabled die penalty"].asBool())
        flags |= fb::game::map::options::DISABLE_DIE_PENALTY;

    return flags;
}

bool fb::game::container::map::load_data(uint16_t id, std::vector<char>& buffer)
{
    char                    fname[256];
    sprintf(fname, "maps/%06d.map", id);

    std::ifstream           map_file(fname, std::ios::binary);
    if(map_file.is_open() == false)
        return false;

    buffer = std::vector<char>(std::istreambuf_iterator<char>(map_file), { });
    map_file.close();

    return true;
}

bool fb::game::container::map::load_blocks(uint16_t id, Json::Value& buffer)
{
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

fb::game::container::item::item()
{ }

fb::game::container::item::~item()
{
    for(auto x : *this)
        delete x.second;
}

fb::game::item::master* fb::game::container::item::create(uint32_t id, const Json::Value& data)
{
    std::string         types         = CP949(data["type"].asString(), PLATFORM::Windows);
    std::string         name          = CP949(data["name"].asString(), PLATFORM::Windows);
    uint16_t            icon          = data["icon"].asInt() + 0xBFFF;
    uint8_t             color         = data["color"].asInt();

    auto                price         = data["price"].asInt();
    auto                condition     = to_condition(data);
    auto                penalty       = to_penalty(CP949(data["death penalty"].asString(), PLATFORM::Windows));
    auto                capacity      = std::max(1, data["capacity"].asInt());
    auto                trade         = fb::game::item::trade(data["trade"]["enabled"].asBool());
    auto                storage       = fb::game::item::storage(data["storage"]["enabled"].asBool(), data["storage"]["price"].asInt());
    auto                desc          = CP949(data["desc"].asString(), PLATFORM::Windows);
    auto                script_active = CP949(data["script"]["active"].asString(), PLATFORM::Windows);
    assert_script(script_active, "function\\s+(on_active)\\(\\w+,\\s*\\w+\\)", "on_active", [](const std::string& script) 
    {
#if defined DEBUG | defined _DEBUG
        auto out = std::ofstream(script, std::ios_base::app);
        out << "\n\nfunction on_active(me, item)\n"
            << "\n"
            << "end";
#endif
    });
    
    if(types == "stuff")
    {
        return new fb::game::item::master(name, 
            icon, 
            color, 
            id, 
            price, 
            condition,
            penalty,
            capacity,
            trade,
            storage,
            desc,
            script_active);
    }

    if(types == "consume")
    {
        std::string     bundle_type = CP949(data["bundle type"].asString(), PLATFORM::Windows);   // package Ex) 동동주
                                                                                                  // bundle  Ex) 도토리
        if (bundle_type == "package")
            return new fb::game::pack::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, capacity);
        else
            return new fb::game::consume::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active);
    }

    auto&               option         = data["equipment option"];
    auto                look           = option["look"].asInt();
    auto                durability     = option["durability"].asInt();
    auto                repair         = fb::game::equipment::repair(option["repair"]["enabled"].asBool(), option["repair"]["price"].asDouble());
    auto                rename         = fb::game::equipment::rename(option["rename"]["enabled"].asBool(), option["rename"]["price"].asInt());
    auto                hit            = option["hit"].asInt();
    auto                damage         = option["damage"].asInt();
    auto                strength       = option["strength"].asInt();
    auto                intelligence   = option["intelligence"].asInt();
    auto                dexteritry     = option["dexteritry"].asInt();
    auto                base_hp        = option["base_hp"].asInt();
    auto                base_mp        = option["base_mp"].asInt();
    auto                hp_percent     = option["hp percent"].asInt();
    auto                mp_percent     = option["mp percent"].asInt();
    auto                healing_cycle  = option["healing_cycle"].asInt();
    auto                defensive      = fb::game::defensive(option["defensive"]["physical"].asInt(), option["defensive"]["magical"].asInt());
    auto                script_dress   = CP949(data["script"]["dress"].asString(), PLATFORM::Windows);
    assert_script(script_dress, "function\\s+(on_dress)\\(\\w+,\\s*\\w+\\)", "on_dress", [](const std::string& script) 
    {
#if defined DEBUG | defined _DEBUG
        auto out = std::ofstream(script, std::ios_base::app);
        out << "\n\nfunction on_dress(me, item)\n"
            << "\n"
            << "end";
#endif
    });

    auto                script_undress = CP949(data["script"]["undress"].asString(), PLATFORM::Windows);
    assert_script(script_undress, "function\\s+(on_undress)\\(\\w+,\\s*\\w+\\)", "on_undress", [](const std::string& script) 
    {
#if defined DEBUG | defined _DEBUG
        auto out = std::ofstream(script, std::ios_base::app);
        out << "\n\nfunction on_undress(me, item)\n"
            << "\n"
            << "end";
#endif
    });

    
    if(types == "weapon")
    {
        auto damage_range_min = fb::game::range32_t(option["damage range"]["small"]["min"].asInt(), option["damage range"]["small"]["max"].asInt());
        auto damage_range_max = fb::game::range32_t(option["damage range"]["large"]["min"].asInt(), option["damage range"]["large"]["max"].asInt());
        auto sound = option["sound"].asInt();
        auto spell = CP949(option["spell"].asString(), PLATFORM::Windows);
        return new fb::game::weapon::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, look, durability, repair, rename, script_dress, script_undress, hit, damage, strength, intelligence, dexteritry, base_hp, base_mp, hp_percent, mp_percent, healing_cycle, defensive, damage_range_min, damage_range_max, sound, spell);
    }

    if(types == "armor")
        return new fb::game::armor::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, look, durability, repair, rename, script_dress, script_undress, hit, damage, strength, intelligence, dexteritry, base_hp, base_mp, hp_percent, mp_percent, healing_cycle, defensive);

    if(types == "helmet")
        return new fb::game::helmet::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, look, durability, repair, rename, script_dress, script_undress, hit, damage, strength, intelligence, dexteritry, base_hp, base_mp, hp_percent, mp_percent, healing_cycle, defensive);

    if(types == "shield")
        return new fb::game::shield::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, look, durability, repair, rename, script_dress, script_undress, hit, damage, strength, intelligence, dexteritry, base_hp, base_mp, hp_percent, mp_percent, healing_cycle, defensive);

    if(types == "ring")
        return new fb::game::ring::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, look, durability, repair, rename, script_dress, script_undress, hit, damage, strength, intelligence, dexteritry, base_hp, base_mp, hp_percent, mp_percent, healing_cycle, defensive);

    if(types == "auxiliary")
        return new fb::game::auxiliary::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, look, durability, repair, rename, script_dress, script_undress, hit, damage, strength, intelligence, dexteritry, base_hp, base_mp, hp_percent, mp_percent, healing_cycle, defensive);

    if(types == "bow")
        return new fb::game::bow::master(name, icon, color, id, price, condition, penalty, capacity, trade, storage, desc, script_active, look, durability, repair, rename, script_dress, script_undress, hit, damage, strength, intelligence, dexteritry, base_hp, base_mp, hp_percent, mp_percent, healing_cycle, defensive);

    return nullptr;
}

fb::game::item::conditions fb::game::container::item::to_condition(const Json::Value& data)
{
    auto sex = fb::game::sex::BOTH;

    if(data["condition"].isMember("sex"))
    {
        auto value = CP949(data["condition"]["sex"].asString(), PLATFORM::Windows);
        if(value == "man")
            sex = fb::game::sex::MAN;
        else if(value == "woman")
            sex = fb::game::sex::WOMAN;
        else
            throw std::runtime_error(fb::game::message::assets::INVALID_SEX);
    }

    return fb::game::item::conditions
    (
        data["condition"].isMember("level")        ? data["condition"]["level"].asInt()        : 0,
        data["condition"].isMember("strength")     ? data["condition"]["strength"].asInt()     : 0,
        data["condition"].isMember("dexteritry")   ? data["condition"]["dexteritry"].asInt()   : 0,
        data["condition"].isMember("intelligence") ? data["condition"]["intelligence"].asInt() : 0,
        data["condition"].isMember("class")        ? data["condition"]["class"].asInt()        : 0,
        data["condition"].isMember("promotion")    ? data["condition"]["promotion"].asInt()    : 0,
        sex
    );
}

fb::game::item::penalties fb::game::container::item::to_penalty(const std::string& penalty)
{
    if(penalty.empty())
        return fb::game::item::penalties::NONE;

    if(penalty == "drop")
        return fb::game::item::penalties::DROP;

    if(penalty == "destroy")
        return fb::game::item::penalties::DESTRUCTION;

    throw std::runtime_error(fb::game::message::assets::INVALID_DEATH_PENALTY);
}

fb::game::container::map::map()
{ }

fb::game::container::map::~map()
{
    for(auto& x : *this)
        delete x.second;
}

bool fb::game::container::map::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex       = std::make_unique<std::mutex>();
    auto&                       config      = fb::config::get();
    auto                        group_mine  = config["group"].asUInt();
    auto                        count       = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                id          = std::stoi(key.asString());
            auto                name        = CP949(data["name"].asString(), PLATFORM::Windows);
            // Parse common data
            uint16_t            parent      = data["parent"].asInt();
            uint8_t             bgm         = data["bgm"].asInt();
            auto                effect      = this->to_effect(CP949(data["effect"].asString(), PLATFORM::Windows));
            auto                option      = this->to_option(data);
            int                 group       = data["host group"].asUInt();
            auto                required    = (group == group_mine);

            std::vector<char>   binary;
            Json::Value         blocks;
            if(required)
            {
                // Load binary
                if(this->load_data(id, binary) == false)
                    throw std::runtime_error(fb::game::message::assets::CANNOT_LOAD_MAP_DATA);

                // Load blocks
                if(this->load_blocks(id, blocks) == false)
                    throw std::runtime_error(fb::game::message::assets::CANNOT_LOAD_MAP_BLOCK);
            }

            auto                map = new fb::game::map(id, parent, bgm, name, option, effect, group, binary.data(), binary.size());
            for (const auto& block : blocks)
                map->block(block["x"].asInt(), block["y"].asInt(), true);

            {   auto _ = std::lock_guard<std::mutex>(*mutex);

                std::map<uint16_t, fb::game::map*>::insert(std::make_pair(id, map));
                callback((map->name()), percentage);
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        { 
            auto _ = std::lock_guard(*mutex);
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
    );

    complete(count);
    return true;
}

bool fb::game::container::map::load_warps(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex       = std::make_unique<std::mutex>();

    auto                        count       = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                map_id      = std::stoi(key.asString());
            auto                map         = fb::game::table::maps[map_id];
            if (map == nullptr)
                return;

            auto _ = std::lock_guard(*mutex);
            for(auto& js_warp : data)
            {
                const point16_t before(js_warp["before"]["x"].asInt(), js_warp["before"]["y"].asInt());

                if(js_warp.isMember("world"))
                {
                    auto        widx        = js_warp["world"]["wm"].asInt();
                    auto        world       = fb::game::table::worlds[widx];

                    auto        gidx        = js_warp["world"]["group"].asInt();
                    auto&       group       = (*world)[gidx];

                    auto        oidx        = js_warp["world"]["offset"].asInt();
                    auto&       offset      = (*group)[oidx];
                    map->push_warp(offset.get(), before);
                }
                else
                {
                    const point16_t after(js_warp["after"]["x"].asInt(), js_warp["after"]["y"].asInt());
                    const range8_t  condition(js_warp["limit"]["min"].asInt(), js_warp["limit"]["max"].asInt());

                    auto        next_map_id = js_warp["to"].asInt();
                    auto        next_map    = fb::game::table::maps[next_map_id];
                    map->push_warp(next_map, before, after, condition);
                }
            }

            callback(map->name(), percentage);
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            auto                map_id      = std::stoi(key.asString());
            auto                map         = fb::game::table::maps[map_id];
            error(map->name(), e);
        }
    );

    complete(count);
    return true;
}

bool fb::game::container::item::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex = std::make_unique<std::mutex>();
    auto                        count = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                item = (fb::game::item::master*)nullptr;
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);

            // Create x core
            item = this->create(std::stoi(key.asString()), data);
            uint16_t            id = std::stoi(key.asString());
            {
                auto _ = std::lock_guard(*mutex);
                std::map<uint16_t, fb::game::item::master*>::insert(std::make_pair(id, item));
                callback(name, percentage);
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            auto                        name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::npc::npc()
{ }

fb::game::container::npc::~npc()
{
    for(auto& x : *this)
        delete x.second;
}

bool fb::game::container::npc::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex   = std::make_unique<std::mutex>();
    auto                        count   = fb::table::load
    (
        path, 
        [&](Json::Value& key, Json::Value& data, double percentage)
        {
            uint32_t            id      = std::stoi(key.asString());
            auto                name    = CP949(data["name"].asString(), PLATFORM::Windows);
            uint16_t            look    = data["look"].asInt() + 0x7FFF;
            uint8_t             color   = data["color"].asInt();
            auto                script  = CP949(data["script"].asString(), PLATFORM::Windows);
            assert_script(script, "function\\s+(on_interact)\\(\\w+,\\s*\\w+\\)", "on_interact", [](const std::string& script) 
            {
#if defined DEBUG | defined _DEBUG
                auto out = std::ofstream(script, std::ios_base::app);
                out << "\n\nfunction on_interact(me, npc)\n"
                    << "\n"
                    << "end";
#endif
            });

            {
                auto _ = std::lock_guard(*mutex);
                std::map<uint16_t, fb::game::npc::master*>::insert(std::make_pair(id, new fb::game::npc::master(name, look, color, script)));
                callback(name, percentage);
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
        );

    complete(count);
    return true;
}

bool fb::game::container::npc::load_spawn(const std::string& path, fb::game::context& context, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex           = std::make_unique<std::mutex>();
    auto&                       config          = fb::config::get();
    auto                        current_host    = config["id"].asString();
    auto                        count           = fb::table::load
    (
        path, 
        [&](Json::Value& key, Json::Value& data, double percentage)
        {
            auto                map_id          = std::stoi(key.asString());
            auto                map             = fb::game::table::maps[map_id];
            if (map == nullptr)
                return;

            for (auto& spawn : data)
            {
                auto            npc_id          = spawn["npc"].asInt();
                auto            core            = fb::game::table::npcs[npc_id];
                if (core == nullptr)
                    continue;

                point16_t       position(spawn["position"]["x"].asInt(), spawn["position"]["y"].asInt());
                auto            direction_str   = CP949(spawn["direction"].asString(), PLATFORM::Windows);
                auto            direction       = fb::game::direction::BOTTOM;
                if (direction_str == "top")
                    direction = fb::game::direction::TOP;
                else if (direction_str == "right")
                    direction = fb::game::direction::RIGHT;
                else if (direction_str == "bottom")
                    direction = fb::game::direction::BOTTOM;
                else if (direction_str == "left")
                    direction = fb::game::direction::LEFT;
                else
                    throw std::runtime_error(fb::game::message::assets::INVALID_NPC_DIRECTION);

                auto                cloned      = core->make<fb::game::npc>(context);
                cloned->direction(direction);
                
                {
                    auto _ = std::lock_guard(*mutex);
                    cloned->map(map, position);
                    callback(map->name(), percentage);
                }
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            auto                map_id = std::stoi(key.asString());
            auto                map    = fb::game::table::maps[map_id];
            error(map->name(), e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::mob::mob()
{ }

fb::game::container::mob::~mob()
{
    for(auto& x : *this)
        delete x.second;
}

fb::game::mob::sizes fb::game::container::mob::to_size(const std::string& size)
{
    if(size == "small")
        return fb::game::mob::sizes::SMALL;

    if(size == "large")
        return fb::game::mob::sizes::LARGE;

    throw std::runtime_error(fb::game::message::assets::INVALID_MOB_SIZE);
}

fb::game::mob::offensive_type fb::game::container::mob::to_offensive(const std::string& offensive)
{
    if(offensive == "containment")
        return fb::game::mob::offensive_type::CONTAINMENT;

    if(offensive == "counter")
        return fb::game::mob::offensive_type::COUNTER;

    if(offensive == "none")
        return fb::game::mob::offensive_type::NONE;

    if(offensive == "non move")
        return fb::game::mob::offensive_type::NON_MOVE;

    if(offensive == "run away")
        return fb::game::mob::offensive_type::RUN_AWAY;

    throw std::runtime_error(fb::game::message::assets::INVALID_MOB_OFFENSIVE);
}

bool fb::game::container::mob::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex = std::make_unique<std::mutex>();
    auto                        count = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            uint16_t            id = std::stoi(key.asString());
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);

            uint16_t            look    = data["look"].asInt() + 0x7FFF;
            uint8_t             color   = data["color"].asInt();
            uint32_t            base_hp = data["hp"].asInt();
            uint32_t            base_mp = data["mp"].asInt();

            auto                script_attack = CP949(data["script"]["attack"].asString(), PLATFORM::Windows);
            assert_script(script_attack, "function\\s+(on_attack)\\(\\w+,\\s*\\w+\\)", "on_attack", [](const std::string& script) 
            {
#if defined DEBUG | defined _DEBUG
                auto out = std::ofstream(script, std::ios_base::app);
                out << "\n\nfunction on_attack(me, you)\n"
                    << "    return false\n"
                    << "end";
#endif
            });

            auto                script_die = CP949(data["script"]["die"].asString(), PLATFORM::Windows);
            assert_script(script_die, "function\\s+(on_die)\\(\\w+,\\s*\\w+\\)", "on_die", [](const std::string& script) 
            {
#if defined DEBUG | defined _DEBUG
                auto out = std::ofstream(script, std::ios_base::app);
                out << "\n\nfunction on_die(me, you)\n"
                    << "\n"
                    << "end";
#endif
            });

            auto                mob = new fb::game::mob::master(name, look, color, fb::game::defensive(data["defensive"]["physical"].asInt(), data["defensive"]["magical"].asInt()), base_hp, base_mp, data["experience"].asInt(), fb::game::mob::damage(data["damage"]["min"].asInt(), data["damage"]["max"].asInt()), this->to_offensive(CP949(data["offensive"].asString(), PLATFORM::Windows)), this->to_size(CP949(data["size"].asString(), PLATFORM::Windows)), std::chrono::milliseconds(data["speed"].asInt()), script_attack, script_die);
            {
                auto _ = std::lock_guard(*mutex);
                std::map<uint16_t, fb::game::mob::master*>::insert(std::make_pair(id, mob));
                callback(name, percentage);
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
    );

    complete(count);
    return true;
}

bool fb::game::container::mob::load_drops(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex       = std::make_unique<std::mutex>();
    char                        buffer[256] = {0,};
    auto                        count       = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                name        = CP949(key.asString(), PLATFORM::Windows);
            auto                core        = fb::game::table::mobs.name2mob(name);
            if (core == nullptr)
            {
                sprintf(buffer, "%s (%s)", fb::game::message::assets::INVALID_MOB_NAME, name.c_str());
                throw std::runtime_error(buffer);
            }

            for(auto& x : data)
            {
                float       percentage      = x["percentage"].asFloat();
                auto        item_name       = CP949(x["item"].asString(), PLATFORM::Windows);
                auto        item_core       = fb::game::table::items.name2item(item_name);

                if (item_core == nullptr)
                {
                    sprintf(buffer, "%s (%s)", fb::game::message::assets::INVALID_ITEM_NAME, item_name.c_str());
                    throw std::runtime_error(buffer);
                }

                {
                    auto _ = std::lock_guard(*mutex);
                    core->push_drop(fb::game::mob::drop(item_core, percentage));
                }
            }

            {
                auto _ = std::lock_guard(*mutex);
                callback(name, percentage);
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            auto                name = CP949(key.asString(), PLATFORM::Windows);
            error("", e);
        }
    );

    complete(count);
    return true;
}

bool fb::game::container::mob::load_spawn(const std::string& path, fb::game::context& context, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex   = std::make_unique<std::mutex>();
    auto                        count   = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                map_id  = std::stoi(key.asString());
            auto                map     = fb::game::table::maps[map_id];
            if (map == nullptr)
                return;

            for (auto& spawn : data)
            {
                auto            mob_id  = spawn["mob"].asInt();
                auto            core    = fb::game::table::mobs[mob_id];
                if (core == nullptr)
                    continue;

                uint16_t        x0      = spawn["area"]["x0"].asInt();
                uint16_t        x1      = spawn["area"]["x1"].asInt();
                uint16_t        y0      = spawn["area"]["y0"].asInt();
                uint16_t        y1      = spawn["area"]["y1"].asInt();
                uint16_t        count   = spawn["count"].asInt();
                uint32_t        rezen   = spawn["rezen time"].asInt();

                {
                    auto _ = std::lock_guard(*mutex);
                    for (int i = 0; i < count; i++)
                    {
                        auto        mob = core->make<fb::game::mob>(context);
                        mob->spawn_point(x0, y0);
                        mob->spawn_size(x1, y1);
                        mob->respawn_time(std::chrono::seconds(rezen));
                        mob->map(map);
                    }
                }
            }

            {
                auto _ = std::lock_guard(*mutex);
                callback(map->name(), percentage);
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            auto                map_id  = std::stoi(key.asString());
            auto                map     = fb::game::table::maps[map_id];
            error(map->name(), e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::spell::spell()
{ }

fb::game::container::spell::~spell()
{
    for(auto& x : *this)
        delete x.second;
}

bool fb::game::container::spell::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex = std::make_unique<std::mutex>();
    auto                        count = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            uint16_t            id   = std::stoi(key.asString());
            const auto          name = CP949(data["name"].asString(), PLATFORM::Windows);
            uint8_t             type = data["type"].asInt();

            std::string         cast;
            if (data.isMember("cast"))
            {
                cast = CP949(data["cast"].asString(), PLATFORM::Windows);
                assert_script(cast, "function\\s+(on_cast)\\(\\w+,\\s*.*\\)", "on_cast", [](const std::string& script) 
                {
#if defined DEBUG | defined _DEBUG
                    auto out = std::ofstream(script, std::ios_base::app);
                    out << "\n\nfunction on_cast(me, spell)\n"
                        << "\n"
                        << "end";
#endif
                });
                if(std::filesystem::exists(cast) == false)
                    cast = "";
            }

            std::string         uncast;
            if (data.isMember("uncast"))
            {
                uncast = CP949(data["uncast"].asString(), PLATFORM::Windows);
                assert_script(uncast, "function\\s+(on_uncast)\\(\\w+,\\s*\\w+\\)", "on_uncast", [](const std::string& script) 
                {
#if defined DEBUG | defined _DEBUG
                    auto out = std::ofstream(script, std::ios_base::app);
                    out << "\n\nfunction on_uncast(me, spell)\n"
                        << "\n"
                        << "end";
#endif
                });
                if(std::filesystem::exists(uncast) == false)
                    uncast = "";
            }

            std::string         concast;
            if (data.isMember("concast"))
            {
                concast = CP949(data["concast"].asString(), PLATFORM::Windows);
                assert_script(concast, "function\\s+(on_concast)\\(\\w+,\\s*\\w+\\)", "on_concast", [](const std::string& script) 
                {
#if defined DEBUG | defined _DEBUG
                    auto out = std::ofstream(script, std::ios_base::app);
                    out << "\n\nfunction on_concast(me, spell)\n"
                        << "\n"
                        << "end";
#endif
                });
            }

            std::string         message;
            if (data.isMember("message"))
            {
                message = CP949(data["message"].asString(), PLATFORM::Windows);
            }

            {
                auto _ = std::lock_guard(*mutex);
                std::map<uint16_t, fb::game::spell*>::insert(std::make_pair(id, new fb::game::spell(id, fb::game::spell::types(type), name, cast, uncast, concast, message)));
                callback(name, percentage);
            }
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto _ = std::lock_guard(*mutex);
            const auto          name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::cls::cls()
{ }

fb::game::container::cls::~cls()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::cls::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        count = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            uint8_t             id      = key.asInt();      // class key
            auto                classes = new class_data();

            auto&               levels  = data["levels"];
            for (auto i2 = levels.begin(); i2 != levels.end(); i2++)
            {
                uint32_t        key       = i2.key().asInt();
                auto&           data      = *i2;
                classes->push(fb::game::ability(data["strength"].asInt(),
                                                data["intelligence"].asInt(),
                                                data["dexteritry"].asInt(),
                                                data["hp"].asInt(),
                                                data["mp"].asInt(),
                                                (uint32_t)data["exp"].asInt64()));
            }

            for (auto& promotion : data["promotions"])
            {
                classes->push(CP949(promotion.asString(), PLATFORM::Windows));
            }


            std::vector<fb::game::class_data*>::push_back(classes);
            callback(std::to_string(id), percentage);
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            uint8_t             id = key.asInt();
            error(std::to_string(id), e);
        },
        false
    );

    complete(count);
    return true;
}

fb::game::container::mix::mix()
{ }

fb::game::container::mix::~mix()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::mix::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        count   = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                id      = key.asInt();
            auto                itemmix = new fb::game::itemmix((float)data["percentage"].asDouble());

            for(auto& require : data["require"])
            {
                auto            item    = fb::game::table::items.name2item(CP949(require["item"].asString(), PLATFORM::Windows));
                uint32_t        count   = require["count"].asInt();
                itemmix->require_add(item, count);
            }

            for(auto& success: data["success"])
            {
                auto            item    = fb::game::table::items.name2item(CP949(success["item"].asString(), PLATFORM::Windows));
                uint32_t        count   = success["count"].asInt();
                itemmix->success_add(item, count);
            }

            for(auto& failed: data["failed"])
            {
                auto            item    = fb::game::table::items.name2item(CP949(failed["item"].asString(), PLATFORM::Windows));
                uint32_t        count   = failed["count"].asInt();
                itemmix->failed_add(item, count);
            }

            std::vector<fb::game::itemmix*>::push_back(itemmix);
            //callback(std::to_string(key), percentage);
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            //auto            key = key.asInt();
            //error(std::to_string(key), e);
        },
        false
    );

    complete(count);
    return true;
}

fb::game::itemmix* fb::game::container::mix::find(const std::vector<fb::game::item*>& items)
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&items](itemmix* mx)
        {
            if(mx->require.size() != items.size())
                return false;

            return mx->matched(items);
        });

    return i != this->end() ? *i : nullptr;
}

fb::game::container::door::door()
{ }

fb::game::container::door::~door()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::door::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        count   = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                id      = key.asInt();
            auto                created = new fb::game::door::master();
            for(auto& e : data)
            {
                auto            open    = e["open"].asInt();
                auto            close   = e["close"].asInt();
                created->push_back(fb::game::door::element(open, close));
            }

            std::vector<fb::game::door::master*>::push_back(created);
            callback(std::to_string(id), percentage);
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            auto                id      = key.asInt();
            error(std::to_string(id), e);
        },
        false
    );

    complete(count);
    return true;
}

fb::game::container::worlds::worlds()
{ }

fb::game::container::worlds::~worlds()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::worlds::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        count   = fb::table::load
    (
        path, 
        [&] (Json::Value& key, Json::Value& data, double percentage)
        {
            auto                name    = CP949(data["name"].asString(), PLATFORM::Windows);
            auto                world   = new fb::game::wm::world(name);
            for(auto& js_group : data["world"])
            {
                auto            group   = new fb::game::wm::group();
                for(auto& js_offset : js_group)
                {
                    auto        id      = CP949(js_offset["id"].asString(), PLATFORM::Windows);

                    group->push(new fb::game::wm::offset
                    (
                        id, 
                        CP949(js_offset["name"].asString(), PLATFORM::Windows),
                        point16_t
                        (
                            js_offset["position"]["x"].asInt(), 
                            js_offset["position"]["y"].asInt()
                        ), 
                        fb::game::wm::destination
                        (
                            point16_t
                            (
                                js_offset["destination"]["x"].asInt(), 
                                js_offset["destination"]["y"].asInt()
                            ), 
                            table::maps[js_offset["destination"]["map"].asInt()]
                        )
                    ));
                }

                world->push(group);
            }

            std::vector<fb::game::wm::world*>::push_back(world);
            callback(name, percentage);
        },
        [&] (Json::Value& key, Json::Value& data, const std::string& e)
        {
            error(key.asString(), e);
        },
        false
    );

    complete(count);
    return true;
}

int fb::game::container::worlds::find(const std::string& id) const
{
    for(int i = 0; i < this->size(); i++)
    {
        auto world = this->at(i);

        const auto found = std::find_if
        (
            world->cbegin(), world->cend(),
            [&] (const std::unique_ptr<fb::game::wm::group>& group)
            {
                return std::find_if
                (
                    group->cbegin(), group->cend(),
                    [&] (const std::unique_ptr<fb::game::wm::offset>& x)
                    {
                        return x->id == id;
                    }
                ) != group->end();
            }
        );

        if(found != world->cend())
            return i;
    }

    return -1;
}