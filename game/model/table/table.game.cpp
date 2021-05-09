#include "model/session/session.h"
#include "table.game.h"

std::vector<fb::game::clan*>            fb::game::table::clans;
fb::game::board                         fb::game::table::board;
fb::game::container::map                fb::game::table::maps;
fb::game::container::worlds             fb::game::table::worlds;
fb::game::container::item               fb::game::table::items;
fb::game::container::npc                fb::game::table::npcs;
fb::game::container::mob                fb::game::table::mobs;
fb::game::container::spell              fb::game::table::spells;
fb::game::container::cls                fb::game::table::classes;
fb::game::container::mix                fb::game::table::mixes;
fb::game::container::door               fb::game::table::doors;

fb::game::table::table()
{}

fb::game::table::~table()
{}

fb::game::map* fb::game::container::map::name2map(const std::string& name)
{
    for(auto pair : *this)
    {
        if(pair.second->name() == name)
            return pair.second;
    }

    return nullptr;
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
            return pair.second->name() == name;
        });

    return i != this->end() ? (*i).second : nullptr;
}

fb::game::mob::master* fb::game::container::mob::name2mob(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](std::pair<uint16_t, fb::game::mob::master*> pair)
        {
            return pair.second->name() == name;
        });

    return i != this->end() ? (*i).second : nullptr;
}

fb::game::item::master* fb::game::container::item::name2item(const std::string& name)
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](std::pair<uint16_t, fb::game::item::master*> pair)
        {
            return pair.second->name() == name;
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

        return this->operator[](class_id)->level_abilities[level]->exp;
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

    return fb::game::map::effects::NO_EFFECT;
}

fb::game::map::options fb::game::container::map::to_option(const Json::Value& data)
{
    uint8_t                 flags = fb::game::map::options::NO_OPTION;
    auto                    option = data["option"];

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

    return fb::game::map::options(flags);
}

bool fb::game::container::map::load_data(uint16_t id, std::vector<char>& buffer)
{
    char                    fname[256];
    sprintf(fname, "maps/%06d.map", id);

    std::ifstream           map_file(fname, std::ios::binary);
    if(map_file.is_open() == false)
        return false;

    buffer = std::vector<char>(std::istreambuf_iterator<char>(map_file), {});
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
{}

fb::game::container::item::~item()
{
    for(auto x : *this)
        delete x.second;
}

fb::game::item::master* fb::game::container::item::create(uint32_t id, const Json::Value& data)
{
    std::string         types           = CP949(data["type"].asString(), PLATFORM::Windows);
    std::string         name            = CP949(data["name"].asString(), PLATFORM::Windows);
    uint16_t            icon            = data["icon"].asInt() + 0xBFFF;
    uint8_t             color           = data["color"].asInt();

    if(types == "stuff")
    {
        uint16_t        capacity        = std::max(data["capacity"].asInt(), 1);
        return new fb::game::item::master(id, name, icon, color, capacity);
    }

    if(types == "consume")
    {
        std::string     bundle_type = CP949(data["bundle type"].asString(), PLATFORM::Windows);   // package Ex) 동동주
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

fb::game::item::item_limit fb::game::container::item::to_limit(const Json::Value& data)
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
        auto sex = CP949(data["limit"]["sex"].asString(), PLATFORM::Windows);
        if(sex == "man")
            limit.sex = fb::game::sex::MAN;
        else if(sex == "woman")
            limit.sex = fb::game::sex::WOMAN;
        else
            throw std::runtime_error(fb::game::message::assets::INVALID_SEX);
    }

    return limit;
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
{}

fb::game::container::map::~map()
{
    for(auto x : *this)
        delete x.second;
}

bool fb::game::container::map::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto                        mutex = std::make_unique<std::mutex>();
    auto                        queue = std::queue<std::pair<uint16_t, std::unique_ptr<Json::Value>>>();

    auto&                       config = fb::config::get();
    auto                        group_mine = config["group"].asUInt();
    auto                        count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            // enqueue all meta files
mutex->lock();
            queue.push
            (
                std::make_pair
                (
                    std::stoi(i.key().asString()), 
                    std::make_unique<Json::Value>(*i)
                )
            );
mutex->unlock();
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        { }
    );

    auto                                    num_threads = std::thread::hardware_concurrency();
    auto                                    threads = std::make_unique<std::unique_ptr<std::thread>[]>(num_threads);
#if defined DEBUG | defined _DEBUG
    auto                                    begin = std::chrono::steady_clock::now();
#endif
    for(int i = 0; i < num_threads; i++)
    {
        threads[i] = std::make_unique<std::thread>
        (
            [&] ()
            {
                // dequeue all meta files
                while(true)
                {
                    auto                    key = uint16_t(0);
                    auto                    data = Json::Value();

                    {   auto _ = std::lock_guard<std::mutex>(*mutex);

                        if(queue.empty())
                            break;

                        key  = queue.front().first;
                        data = *queue.front().second;
                        queue.pop();
                    }

                    auto                    name = CP949(data["name"].asString(), PLATFORM::Windows);
                    try
                    {
                        // Parse common data
                        uint16_t            parent = data["parent"].asInt();
                        uint8_t             bgm = data["bgm"].asInt();
                        auto                effect = this->to_effect(CP949(data["effect"].asString(), PLATFORM::Windows));
                        auto                option = this->to_option(data);
                        int                 group = data["host group"].asUInt();
                        auto                required = (group == group_mine);

                        std::vector<char>   binary;
                        Json::Value         blocks;
                        if(required)
                        {
                            // Load binary
                            if(this->load_data(key, binary) == false)
                                throw std::runtime_error(fb::game::message::assets::CANNOT_LOAD_MAP_DATA);

                            // Load blocks
                            if(this->load_blocks(key, blocks) == false)
                                throw std::runtime_error(fb::game::message::assets::CANNOT_LOAD_MAP_BLOCK);
                        }

                        auto                map = new fb::game::map(key, parent, bgm, name, option, effect, group, binary.data(), binary.size());
                        for (const auto& block : blocks)
                            map->block(block["x"].asInt(), block["y"].asInt(), true);

                        {   auto _ = std::lock_guard<std::mutex>(*mutex);

                            std::map<uint16_t, fb::game::map*>::insert(std::make_pair(key, map));
                            
                            auto                percentage = std::map<uint16_t, fb::game::map*>::size() * 100 / double(count);
                            callback((map->name()), percentage);
                        }
                    }
                    catch(std::exception& e)
                    {   auto _ = std::lock_guard<std::mutex>(*mutex);
                        
                        error(name, e.what());
                    }
                }
            }
        );
    }

    for(int i = 0; i < num_threads; i++)
        threads[i]->join();

    complete(count);

#if defined DEBUG | defined _DEBUG
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
#endif
    return true;
}

bool fb::game::container::map::load_warps(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto                map_id = std::stoi(i.key().asString());
            auto                map = fb::game::table::maps[map_id];
            if (map == nullptr)
                return;

            auto            warps = *i;
            for(auto js_warp : *i)
            {
                const point16_t before(js_warp["before"]["x"].asInt(), js_warp["before"]["y"].asInt());

                if(js_warp.isMember("world"))
                {
                    auto world = fb::game::table::worlds[js_warp["world"]["wm"].asInt()];
                    auto& group = (*world)[js_warp["world"]["group"].asInt()];
                    auto& offset = (*group)[js_warp["world"]["offset"].asInt()];

                    map->push_warp(offset.get(), before);
                }
                else
                {
                    const point16_t after(js_warp["after"]["x"].asInt(), js_warp["after"]["y"].asInt());
                    const range8_t  limit(js_warp["limit"]["min"].asInt(), js_warp["limit"]["max"].asInt());

                    auto        next_map_id = js_warp["to"].asInt();
                    auto        next_map = fb::game::table::maps[next_map_id];

                    map->push_warp(next_map, before, after, limit);
                }
            }

            callback(map->name(), percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                map_id = std::stoi(i.key().asString());
            auto                map = fb::game::table::maps[map_id];
            error(map->name(), e);
        }
    );

    complete(count);
    return true;
}

bool fb::game::container::item::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            fb::game::item::master*     item = nullptr;
            auto                        data = *i;
            auto                        name = CP949(data["name"].asString(), PLATFORM::Windows);

            // Create x core
            item = this->create(std::stoi(i.key().asString()), data);

            // Common options
            item->active_script(CP949(data["script"]["active"].asString(), PLATFORM::Windows));
            item->price(data["price"].asInt());
            item->trade(data["trade"]["enabled"].asBool());
            item->entrust_enabled(data["entrust"]["enabled"].asBool());
            item->entrust_price(data["entrust"]["price"].asInt());
            item->desc(CP949(data["desc"].asString(), PLATFORM::Windows));
            item->limit(this->to_limit(data));
            item->penalty(this->to_penalty(CP949(data["death penalty"].asString(), PLATFORM::Windows)));



            // Equipment options
            if (item->attr(fb::game::item::attrs::ITEM_ATTR_EQUIPMENT) && data.isMember("equipment option"))
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
                equipment->dress_script(CP949(data["script"]["dress"].asString(), PLATFORM::Windows));
                equipment->undress_script(CP949(data["script"]["undress"].asString(), PLATFORM::Windows));
            }


            // Weapon options
            if (item->attr(fb::game::item::attrs::ITEM_ATTR_WEAPON))
            {
                auto            option = data["equipment option"];
                auto            weapon = static_cast<fb::game::weapon::master*>(item);

                weapon->damage_small(option["damage range"]["small"]["min"].asInt(), option["damage range"]["small"]["max"].asInt());
                weapon->damage_large(option["damage range"]["large"]["min"].asInt(), option["damage range"]["large"]["max"].asInt());
                weapon->sound(option["sound"].asInt());
                weapon->spell(CP949(option["spell"].asString(), PLATFORM::Windows));
            }


            uint16_t            id = std::stoi(i.key().asString());
            std::map<uint16_t, fb::game::item::master*>::insert(std::make_pair(id, item));

            callback(name, percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                        data = *i;
            auto                        name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::npc::npc()
{}

fb::game::container::npc::~npc()
{
    for(auto x : *this)
        delete x.second;
}

bool fb::game::container::npc::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&](Json::Value::iterator& i, double percentage)
        {
            auto                data = *i;
            uint32_t            id = std::stoi(i.key().asString());
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);

            uint16_t        look = data["look"].asInt() + 0x7FFF;
            uint8_t         color = data["color"].asInt();

            std::map<uint16_t, fb::game::npc::master*>::insert(std::make_pair(id, new fb::game::npc::master(name, look, color)));

            callback(name, percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                data = *i;
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
        );

    complete(count);
    return true;
}

bool fb::game::container::npc::load_spawn(const std::string& path, fb::game::npc::listener* listener, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto& config = fb::config::get();
    auto current_host = config["id"].asString();
    auto count = fb::table::load
    (
        path, 
        [&](Json::Value::iterator& i, double percentage)
        {
            auto                map_id = std::stoi(i.key().asString());
            auto                map = fb::game::table::maps[map_id];
            if (map == nullptr)
                return;

            auto                spawns = *i;
            for (auto spawn : spawns)
            {
                auto            npc_id = spawn["npc"].asInt();
                auto            core = fb::game::table::npcs[npc_id];
                if (core == nullptr)
                    continue;

                point16_t       position(spawn["position"]["x"].asInt(), spawn["position"]["y"].asInt());
                auto            script = CP949(spawn["script"].asString(), PLATFORM::Windows);
                auto            direction_str = CP949(spawn["direction"].asString(), PLATFORM::Windows);
                auto            direction = fb::game::direction::BOTTOM;
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

                auto                cloned = new fb::game::npc(core, listener);
                cloned->direction(direction);
                cloned->script(script);
                cloned->map(map, position);
            }

            callback(map->name(), percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                map_id = std::stoi(i.key().asString());
            auto                map = fb::game::table::maps[map_id];
            error(map->name(), e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::mob::mob()
{}

fb::game::container::mob::~mob()
{
    for(auto x : *this)
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
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            uint16_t            id = std::stoi(i.key().asString());
            auto                data = *i;
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);

            uint16_t            look = data["look"].asInt() + 0x7FFF;
            uint8_t             color = data["color"].asInt();
            uint32_t            base_hp = data["hp"].asInt();
            uint32_t            base_mp = data["mp"].asInt();

            auto mob = new fb::game::mob::master(name, look, color, base_hp, base_mp);
            mob->defensive_physical(data["defensive"]["physical"].asInt());
            mob->defensive_magical(data["defensive"]["magical"].asInt());
            mob->experience(data["experience"].asInt());
            mob->damage_min(data["damage"]["min"].asInt());
            mob->damage_max(data["damage"]["max"].asInt());
            mob->offensive(this->to_offensive(CP949(data["offensive"].asString(), PLATFORM::Windows)));
            mob->size(this->to_size(CP949(data["size"].asString(), PLATFORM::Windows)));
            mob->speed(std::chrono::milliseconds(data["speed"].asInt()));
            mob->script_attack(CP949(data["script"]["attack"].asString(), PLATFORM::Windows));
            mob->script_die(CP949(data["script"]["die"].asString(), PLATFORM::Windows));

            std::map<uint16_t, fb::game::mob::master*>::insert(std::make_pair(id, mob));

            callback(name, percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                data = *i;
            auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
    );

    complete(count);
    return true;
}

bool fb::game::container::mob::load_drops(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    char buffer[256] = {0,};
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto                name = CP949(i.key().asString(), PLATFORM::Windows);
            auto                core = fb::game::table::mobs.name2mob(name);
            if (core == nullptr)
            {
                sprintf(buffer, "%s (%s)", fb::game::message::assets::INVALID_MOB_NAME, name.c_str());
                throw std::runtime_error(buffer);
            }

            for(auto x : *i)
            {
                float       percentage = x["percentage"].asFloat();
                auto        item_name = CP949(x["item"].asString(), PLATFORM::Windows);
                auto        item_core = fb::game::table::items.name2item(item_name);

                if (item_core == nullptr)
                {
                    sprintf(buffer, "%s (%s)", fb::game::message::assets::INVALID_ITEM_NAME, item_name.c_str());
                    throw std::runtime_error(buffer);
                }

                core->dropitem_add(item_core, percentage);
            }

            callback(name, percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                name = CP949(i.key().asString(), PLATFORM::Windows);
            error("", e);
        }
    );

    complete(count);
    return true;
}

bool fb::game::container::mob::load_spawn(const std::string& path, fb::game::mob::listener* listener, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto                map_id = std::stoi(i.key().asString());
            auto                map = fb::game::table::maps[map_id];
            if (map == nullptr)
                return;

            auto                spawns = *i;
            for (auto spawn : spawns)
            {
                auto            mob_id = spawn["mob"].asInt();
                auto            core = fb::game::table::mobs[mob_id];
                if (core == nullptr)
                    continue;

                uint16_t        x0 = spawn["area"]["x0"].asInt();
                uint16_t        x1 = spawn["area"]["x1"].asInt();
                uint16_t        y0 = spawn["area"]["y0"].asInt();
                uint16_t        y1 = spawn["area"]["y1"].asInt();
                uint16_t        count = spawn["count"].asInt();
                uint32_t        rezen = spawn["rezen time"].asInt();

                for (int i = 0; i < count; i++)
                {
                    auto        mob = static_cast<fb::game::mob*>(core->make(listener));
                    mob->spawn_point(x0, y0);
                    mob->spawn_size(x1, y1);
                    mob->respawn_time(std::chrono::seconds(rezen));
                    mob->map(map);
                }
            }

            callback(map->name(), percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                map_id = std::stoi(i.key().asString());
            auto                map = fb::game::table::maps[map_id];
            error(map->name(), e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::spell::spell()
{}

fb::game::container::spell::~spell()
{
    for(auto x : *this)
        delete x.second;
}

bool fb::game::container::spell::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            uint16_t            id = std::stoi(i.key().asString());
            const auto          data = (*i);
            const auto          name = CP949(data["name"].asString(), PLATFORM::Windows);

            uint8_t             type = data["type"].asInt();

            std::string         cast, uncast, concast, message;
            if (data.isMember("cast"))
                cast = CP949(data["cast"].asString(), PLATFORM::Windows);

            if (data.isMember("uncast"))
                uncast = CP949(data["uncast"].asString(), PLATFORM::Windows);

            if (data.isMember("concast"))
                concast = CP949(data["concast"].asString(), PLATFORM::Windows);

            if (data.isMember("message"))
                message = CP949(data["message"].asString(), PLATFORM::Windows);

            std::map<uint16_t, fb::game::spell*>::insert(std::make_pair(id, new fb::game::spell(id, fb::game::spell::types(type), name, cast, uncast, concast, message)));
            callback(name, percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            const auto          data = (*i);
            const auto          name = CP949(data["name"].asString(), PLATFORM::Windows);
            error(name, e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::cls::cls()
{
}

fb::game::container::cls::~cls()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::cls::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            uint8_t             id = i.key().asInt();      // class key
            auto                data = new class_data();

            auto                levels = (*i)["levels"];
            for (auto i2 = levels.begin(); i2 != levels.end(); i2++)
            {
                uint32_t        key = i2.key().asInt();
                auto            ability = *i2;
                auto            allocated = new level_ability(ability["strength"].asInt(),
                                                              ability["intelligence"].asInt(),
                                                              ability["dexteritry"].asInt(),
                                                              ability["hp"].asInt(),
                                                              ability["mp"].asInt(),
                                                              (uint32_t)ability["exp"].asInt64());

                data->add_level_ability(allocated);
            }

            for (auto promotion : (*i)["promotions"])
                data->add_promotion(CP949(promotion.asString(), PLATFORM::Windows));


            std::vector<fb::game::class_data*>::push_back(data);

            callback(std::to_string(id), percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            uint8_t             id = i.key().asInt();
            error(std::to_string(id), e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::mix::mix()
{
}

fb::game::container::mix::~mix()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::mix::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto            key = i.key().asInt();
            auto            data = *i;
            auto            itemmix = new fb::game::itemmix((float)data["percentage"].asDouble());

            for(auto require : data["require"])
            {
                auto        item = fb::game::table::items.name2item(CP949(require["item"].asString(), PLATFORM::Windows));
                uint32_t    count = require["count"].asInt();
                itemmix->require_add(item, count);
            }

            for(auto success: data["success"])
            {
                auto        item = fb::game::table::items.name2item(CP949(success["item"].asString(), PLATFORM::Windows));
                uint32_t    count = success["count"].asInt();
                itemmix->success_add(item, count);
            }

            for(auto failed: data["failed"])
            {
                auto        item = fb::game::table::items.name2item(CP949(failed["item"].asString(), PLATFORM::Windows));
                uint32_t    count = failed["count"].asInt();
                itemmix->failed_add(item, count);
            }

            std::vector<fb::game::itemmix*>::push_back(itemmix);
            //callback(std::to_string(key), percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            //auto            key = i.key().asInt();
            //error(std::to_string(key), e);
        }
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
{}

fb::game::container::door::~door()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::door::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto                key = i.key().asInt();
            auto                door = *i;
            auto                created = new fb::game::door::master();
            for(auto& e : door)
            {
                auto            open  = e["open"].asInt();
                auto            close = e["close"].asInt();
                created->push_back(fb::game::door::element(open, close));
            }

            std::vector<fb::game::door::master*>::push_back(created);
            callback(std::to_string(key), percentage);
        },
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                key = i.key().asInt();
            error(std::to_string(key), e);
        }
    );

    complete(count);
    return true;
}

fb::game::container::worlds::worlds()
{
}

fb::game::container::worlds::~worlds()
{
    for(auto x : *this)
        delete x;
}

bool fb::game::container::worlds::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
{
    auto count = fb::table::load
    (
        path, 
        [&] (Json::Value::iterator& i, double percentage)
        {
            auto                js_world = *i;
            auto                name = CP949(js_world["name"].asString(), PLATFORM::Windows);
            auto                world = new fb::game::wm::world(name);
            for(auto js_group : js_world["world"])
            {
                auto            group = new fb::game::wm::group();
                for(auto js_offset : js_group)
                {
                    auto        id = CP949(js_offset["id"].asString(), PLATFORM::Windows);

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
        [&] (Json::Value::iterator& i, const std::string& e)
        {
            auto                key = i.key().asString();
            error(key, e);
        }
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