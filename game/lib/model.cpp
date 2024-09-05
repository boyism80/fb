// #include <fb/game/session.h>
// #include <fb/game/model.h>
// #include <regex>

// using namespace fb::game;

// void assert_script(const std::string& script, const std::string& regex, const std::string& func_name, const std::function<void(const std::string&)>& callback)
// {
//     if(script.empty())
//         return;

//     if(std::filesystem::exists(script) == false)
//     {
//         std::stringstream sstream;
//         sstream << script
//                 << " 파일이 존재하지 않습니다.";

//         callback(script);
//         throw std::runtime_error(sstream.str());
//     }

//     auto in = std::ifstream(script);
//     auto buffer = std::stringstream();
//     buffer << in.rdbuf();

//     auto re = std::regex(regex);
//     auto matches = std::smatch();
//     auto source = buffer.str();
//     if(std::regex_search(source, matches, re) == false)
//     {
//         std::stringstream sstream;
//         sstream << script
//                 << " 파일에 "
//                 << func_name
//                 << " 함수가 존재하지 않습니다.";

//         callback(script);
//         throw std::runtime_error(sstream.str());
//     }
// }

// std::vector<fb::game::clan*> fb::game::old_model::clans;
// container::map               fb::game::old_model::maps;
// container::worlds            fb::game::old_model::worlds;
// container::item              fb::game::old_model::items;
// container::npc               fb::game::old_model::npcs;
// container::mob               fb::game::old_model::mobs;
// container::spell             fb::game::old_model::spells;
// container::cls               fb::game::old_model::classes;
// container::mix               fb::game::old_model::mixes;
// container::door              fb::game::old_model::doors;
// container::board             fb::game::old_model::boards;
// container::pursuit           fb::game::old_model::sell;
// container::pursuit           fb::game::old_model::buy;

// fb::game::old_model::old_model()
// { }

// fb::game::old_model::~old_model()
// { }

// fb::game::map* container::map::name2map(const std::string& name)
// {
//     for(auto& [key, value] : *this)
//     {
//         if(value->name() == name)
//             return value.get();
//     }

//     return nullptr;
// }

// fb::model::npc* container::npc::name2npc(const std::string& name)
// {
//     auto i = std::find_if(this->begin(), this->end(), 
//         [&name](auto& pair)
//         {
//             return pair.second->name == name;
//         });

//     return i != this->end() ? i->second.get() : nullptr;
// }

// fb::model::mob* container::mob::name2mob(const std::string& name)
// {
//     auto i = std::find_if(this->begin(), this->end(), 
//         [&name](auto& pair)
//         {
//             return pair.second->name == name;
//         });

//     return i != this->end() ? i->second.get() : nullptr;
// }

// fb::model::item* container::item::name2item(const std::string& name)
// {
//     auto i = std::find_if(this->begin(), this->end(), 
//         [&name](auto& pair)
//         {
//             return pair.second->name == name;
//         });

//     return i != this->end() ? i->second.get() : nullptr;
// }

// fb::model::spell* container::spell::name2spell(const std::string& name)
// {
//     auto i = std::find_if(this->begin(), this->end(), 
//         [&name](auto& pair)
//         {
//             return pair.second->name == name;
//         });

//     return i != this->end() ? i->second.get() : nullptr;
// }

// const std::string* container::cls::class2name(uint8_t cls, uint8_t promotion)
// {
//     try
//     {
//         return &this->operator[](cls)->promotions[promotion];
//     }
//     catch(std::exception&)
//     {
//         return nullptr;
//     }
// }

// bool container::cls::name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id)
// {
//     for(int i1 = 0; i1 < this->size(); i1++)
//     {
//         for(int i2 = 0; i2 < this->operator[](i1)->promotions.size(); i2++)
//         {
//             if(this->operator[](i1)->promotions[i2] != name)
//                 continue;

//             *class_id = i1;
//             *promotion_id = i2;
//             return true;
//         }
//     }

//     return false;
// }

// uint32_t container::cls::exp(uint8_t class_id, uint8_t level)
// {
//     try
//     {
//         if(class_id == 0 && level >= 5)
//             return 0;

//         if(level > 99)
//             return 0;

//         return this->operator[](class_id)->abilities[level]->exp;
//     }
//     catch(std::exception&)
//     {
//         return 0;
//     }
// }

// fb::game::map::EFFECT container::map::to_effect(const std::string& effect)
// {
//     if(effect == "fire")
//         return MAP_EFFECT_TYPE::FIRE;

//     if(effect == "water")
//         return MAP_EFFECT_TYPE::WATER;

//     return MAP_EFFECT_TYPE::NONE;
// }

// MAP_OPTION container::map::to_option(const Json::Value& data)
// {
//     auto                    flags = MAP_OPTION::NO_OPTION;
//     auto&                   option = data["option"];

//     if(option["build in"].asBool())
//         flags |= MAP_OPTION::BUILD_IN;

//     if(option["enabled talk"].asBool() == false)
//         flags |= MAP_OPTION::DISABLE_TALK;

//     if(option["enabled whisper"].asBool() == false)
//         flags |= MAP_OPTION::DISABLE_WHISPER;

//     if(option["enabled magic"].asBool() == false)
//         flags |= MAP_OPTION::DISABLE_MAGIC;

//     if(option["hunting ground"].asBool())
//         flags |= MAP_OPTION::HUNTING_GROUND;

//     if(option["enabled pk"].asBool())
//         flags |= MAP_OPTION::ENABLE_PK;

//     if(option["enabled die penalty"].asBool())
//         flags |= MAP_OPTION::DISABLE_DIE_PENALTY;

//     return flags;
// }

// bool container::map::load_data(uint16_t id, std::vector<char>& buffer)
// {
//     auto                    fname = fb::format("maps/%06d.map", id);

//     std::ifstream           map_file(fname, std::ios::binary);
//     if(map_file.is_open() == false)
//         return false;

//     buffer = std::vector<char>(std::istreambuf_iterator<char>(map_file), { });
//     map_file.close();

//     return true;
// }

// bool container::map::load_blocks(uint16_t id, Json::Value& buffer)
// {
//     auto                    fname = fb::format("maps/%06d.block", id);
//     std::ifstream           file(fname);
//     if(file.is_open() == false)
//         return false;

//     Json::Reader reader;
//     if(reader.parse(file, buffer) == false)
//         return false;

//     file.close();
//     return true;
// }

// fb::model::item* container::item::create(uint32_t id, const Json::Value& data)
// {
//     std::string         types   = CP949(data["type"].asString(), PLATFORM::Windows);
//     auto                config  = fb::model::item::config
//     {
//         { 
//             .name  = CP949(data["name"].asString(), PLATFORM::Windows),
//             .look  = (uint16_t)(data["icon"].asInt() + 0xBFFF),
//             .color = (uint8_t)data["color"].asInt()
//         },
//         /* id            */ id,
//         /* price         */ (uint32_t)data["price"].asInt(),
//         /* condition     */ to_condition(data),
//         /* penalty       */ to_penalty(CP949(data["death_penalty"].asString(), PLATFORM::Windows)),
//         /* capacity      */ std::max(uint16_t(1), (uint16_t)data["capacity"].asInt()),
//         /* trade         */ data["trade"].asBool(),
//         /* deposit_price */ data["deposit_price"].isNull() ? std::optional<uint32_t>() : std::optional<uint32_t>(data["deposit_price"].asUInt()),
//         /* desc          */ CP949(data["desc"].asString(), PLATFORM::Windows),
//         /* active_script */ CP949(data["script"]["active"].asString(), PLATFORM::Windows)
//     };

    
//     assert_script(config.active_script, "function\\s+(on_active)\\(\\w+,\\s*\\w+\\)", "on_active", [](const auto& script) 
//     {
// #if defined DEBUG | defined _DEBUG
//         auto out = std::ofstream(script, std::ios_base::app);
//         out << "\n\nfunction on_active(me, item)\n"
//             << "\n"
//             << "end";
// #endif
//     });
//     lua::load(config.active_script);
    
//     if(types == "stuff")
//     {
//         return new fb::model::item(config);
//     }

//     if(types == "consume")
//     {
//         std::string     bundle_type = CP949(data["bundle_type"].asString(), PLATFORM::Windows);   // package Ex) 동동주
//                                                                                                   // bundle  Ex) 도토리
//         if (bundle_type == "package")
//         {
//             return new fb::game::pack::model(config);
//         }
//         else
//         {
//             return new fb::game::consume::model(config);
//         }
//     }

//     auto&               option           = data["equipment option"];
//     auto                equipment_config = fb::game::equipment::model::config
//     {
//         config,
//         /* dress          */ (uint16_t)option["look"].asInt(),
//         /* durability     */ (uint16_t)option["durability"].asInt(),
//         /* repair         */ option["repair"].isNull() ? std::optional<double>() : std::optional<double>(option["repair"].asDouble()),
//         /* dress_script   */ CP949(data["script"]["dress"].asString(), PLATFORM::Windows),
//         /* undress_script */ CP949(data["script"]["undress"].asString(), PLATFORM::Windows),
//         /* hit            */ (uint8_t)option["hit"].asInt(),
//         /* damage         */ (uint8_t)option["damage"].asInt(),
//         /* strength       */ (uint8_t)option["strength"].asInt(),
//         /* intelligence   */ (uint8_t)option["intelligence"].asInt(),
//         /* dexteritry     */ (uint8_t)option["dexteritry"].asInt(),
//         /* base_hp        */ (uint32_t)option["base_hp"].asInt(),
//         /* base_mp        */ (uint32_t)option["base_mp"].asInt(),
//         /* hp_percentage  */ (float)option["hp percent"].asInt(),
//         /* mp_percentage  */ (float)option["mp percent"].asInt(),
//         /* healing_cycle  */ (uint8_t)option["healing_cycle"].asInt(),
//         /* defensive      */ fb::game::defensive(option["defensive"]["physical"].asInt(), option["defensive"]["magical"].asInt()),
//     };

//     assert_script(equipment_config.dress_script, "function\\s+(on_dress)\\(\\w+,\\s*\\w+\\)", "on_dress", [](const auto& script) 
//     {
// #if defined DEBUG | defined _DEBUG
//         auto out = std::ofstream(script, std::ios_base::app);
//         out << "\n\nfunction on_dress(me, item)\n"
//             << "\n"
//             << "end";
// #endif
//     });
//     lua::load(equipment_config.dress_script);

//     assert_script(equipment_config.undress_script, "function\\s+(on_undress)\\(\\w+,\\s*\\w+\\)", "on_undress", [](const auto& script) 
//     {
// #if defined DEBUG | defined _DEBUG
//         auto out = std::ofstream(script, std::ios_base::app);
//         out << "\n\nfunction on_undress(me, item)\n"
//             << "\n"
//             << "end";
// #endif
//     });
//     lua::load(equipment_config.undress_script);

    
//     if(types == "weapon")
//     {
//         auto weapon_config = fb::game::weapon::model::config
//         {
//             equipment_config,
//             /* small  */ fb::game::range32_t(option["damage range"]["small"]["min"].asInt(), option["damage range"]["small"]["max"].asInt()),
//             /* large  */ fb::game::range32_t(option["damage range"]["large"]["min"].asInt(), option["damage range"]["large"]["max"].asInt()),
//             /* sound  */ (uint16_t)option["sound"].asInt(),
//             /* spell  */ CP949(option["spell"].asString(), PLATFORM::Windows),
//             /* rename */ option["rename"].isNull() ? std::optional<uint32_t>() : std::optional<uint32_t>(option["rename"].asUInt()),
//         };
//         return new fb::game::weapon::model(weapon_config);
//     }

//     if(types == "armor")
//         return new fb::game::armor::model(equipment_config);

//     if(types == "helmet")
//         return new fb::game::helmet::model(equipment_config);

//     if(types == "shield")
//         return new fb::game::shield::model(equipment_config);

//     if(types == "ring")
//         return new fb::game::ring::model(equipment_config);

//     if(types == "auxiliary")
//         return new fb::game::auxiliary::model(equipment_config);

//     if(types == "bow")
//         return new fb::game::bow::model(equipment_config);

//     return nullptr;
// }

// fb::game::item::conditions container::item::to_condition(const Json::Value& data)
// {
//     auto sex = SEX::BOTH;

//     if(data["condition"].isMember("sex"))
//     {
//         auto value = CP949(data["condition"]["sex"].asString(), PLATFORM::Windows);
//         if(value == "man")
//             sex = SEX::MAN;
//         else if(value == "woman")
//             sex = SEX::WOMAN;
//         else
//             throw std::runtime_error(fb::model::const_value::string::MESSAGE_ASSET_INVALID_SEX);
//     }

//     return fb::game::item::conditions
//     {
//         .level        = data["condition"].isMember("level")        ? (uint8_t)data["condition"]["level"].asInt()        : uint8_t(0),
//         .strength     = data["condition"].isMember("strength")     ? (uint8_t)data["condition"]["strength"].asInt()     : uint8_t(0),
//         .dexteritry   = data["condition"].isMember("dexteritry")   ? (uint8_t)data["condition"]["dexteritry"].asInt()   : uint8_t(0),
//         .intelligence = data["condition"].isMember("intelligence") ? (uint8_t)data["condition"]["intelligence"].asInt() : uint8_t(0),
//         .cls          = data["condition"].isMember("class")        ? (uint8_t)data["condition"]["class"].asInt()        : uint8_t(0),
//         .promotion    = data["condition"].isMember("promotion")    ? (uint8_t)data["condition"]["promotion"].asInt()    : uint8_t(0),
//         .sex          = sex
//     };
// }

// DEATH_PENALTY_TYPE container::item::to_penalty(const std::string& penalty)
// {
//     if(penalty.empty())
//         return DEATH_PENALTY_TYPE::NONE;

//     if(penalty == "drop")
//         return DEATH_PENALTY_TYPE::DROP;

//     if(penalty == "destroy")
//         return DEATH_PENALTY_TYPE::DESTRUCTION;

//     throw std::runtime_error(fb::model::const_value::string::MESSAGE_ASSET_INVALID_DEATH_PENALTY);
// }

// bool container::map::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex       = std::make_unique<std::mutex>();
//     auto&                       config      = fb::config::get();
//     auto                        group_mine  = config["group"].asUInt();
//     auto                        count       = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                id          = std::stoi(key.asString());
//             auto                name        = CP949(data["name"].asString(), PLATFORM::Windows);
//             // Parse common data
//             uint16_t            parent      = data["parent"].asInt();
//             uint8_t             bgm         = data["bgm"].asInt();
//             auto                effect      = this->to_effect(CP949(data["effect"].asString(), PLATFORM::Windows));
//             auto                option      = this->to_option(data);
//             int                 group       = data["host group"].asUInt();
//             auto                required    = (group == group_mine);

//             std::vector<char>   binary;
//             Json::Value         blocks;
//             if(required)
//             {
//                 // Load binary
//                 if(this->load_data(id, binary) == false)
//                     throw std::runtime_error(fb::model::const_value::string::MESSAGE_ASSET_CANNOT_LOAD_MAP_DATA);

//                 // Load blocks
//                 if(this->load_blocks(id, blocks) == false)
//                     throw std::runtime_error(fb::model::const_value::string::MESSAGE_ASSET_CANNOT_LOAD_MAP_BLOCK);
//             }

//             auto                map = new fb::game::map(id, parent, bgm, name, option, effect, group, binary.data(), binary.size());
//             for (const auto& block : blocks)
//                 map->block(block["x"].asInt(), block["y"].asInt(), true);

//             {   auto _ = std::lock_guard(*mutex);

                
//                 this->insert({id, std::unique_ptr<fb::game::map>(map)});
//                 callback((map->name()), percentage);
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         { 
//             auto _ = std::lock_guard(*mutex);
//             auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
//             error(name, e);
//         }
//     );

//     complete(count);
//     return true;
// }

// bool container::map::load_warps(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex       = std::make_unique<std::mutex>();

//     auto                        count       = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                map_id      = std::stoi(key.asString());
//             auto                map         = fb::game::old_model::maps[map_id];
//             if (map == nullptr)
//                 return;

//             auto _ = std::lock_guard(*mutex);
//             for(auto& js_warp : data)
//             {
//                 const point16_t before(js_warp["before"]["x"].asInt(), js_warp["before"]["y"].asInt());

//                 if(js_warp.isMember("world"))
//                 {
//                     auto        widx        = js_warp["world"]["wm"].asInt();
//                     auto        world       = fb::game::old_model::worlds[widx];

//                     auto        gidx        = js_warp["world"]["group"].asInt();
//                     auto&       group       = (*world)[gidx];

//                     auto        oidx        = js_warp["world"]["offset"].asInt();
//                     auto&       offset      = (*group)[oidx];
//                     map->push_warp(offset.get(), before);
//                 }
//                 else
//                 {
//                     const point16_t after(js_warp["after"]["x"].asInt(), js_warp["after"]["y"].asInt());
//                     const range8_t  condition(js_warp["limit"]["min"].asInt(), js_warp["limit"]["max"].asInt());

//                     auto        next_map_id = js_warp["to"].asInt();
//                     auto        next_map    = fb::game::old_model::maps[next_map_id];
//                     map->push_warp(next_map, before, after, condition);
//                 }
//             }

//             callback(map->name(), percentage);
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             auto                map_id      = std::stoi(key.asString());
//             auto                map         = fb::game::old_model::maps[map_id];
//             error(map->name(), e);
//         }
//     );

//     complete(count);
//     return true;
// }

// bool container::item::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex = std::make_unique<std::mutex>();
//     auto                        count = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                item = (fb::model::item*)nullptr;
//             auto                name = CP949(data["name"].asString(), PLATFORM::Windows);

//             // Create x core
//             item = this->create(std::stoi(key.asString()), data);
//             uint16_t            id = std::stoi(key.asString());
//             {
//                 auto _ = std::lock_guard(*mutex);
//                 this->insert({id, std::unique_ptr<fb::model::item>(item)});
//                 callback(name, percentage);
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
//             error(name, e);
//         }
//     );

//     complete(count);
//     return true;
// }

// bool container::npc::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex   = std::make_unique<std::mutex>();
//     auto                        count   = fb::table::load
//     (
//         path, 
//         [&](Json::Value& key, Json::Value& data, double percentage)
//         {
//             uint32_t            id        = std::stoi(key.asString());
//             auto                name      = CP949(data["name"].asString(), PLATFORM::Windows);
//             uint16_t            look      = data["look"].asInt() + 0x7FFF;
//             uint8_t             color     = data["color"].asInt();
//             auto                script    = CP949(data["script"].asString(), PLATFORM::Windows);
//             auto                sells     = std::map<std::string, uint16_t>();
//             auto                buy       = std::optional<uint16_t>();
//             auto                repair    = data["repair"].asBool();
//             auto                hold_money = data["hold_money"].asBool();
//             auto                hold_item = data["hold_item"].asBool();
//             auto                rename    = data["rename"].asBool();

//             if (data.isMember("sell"))
//             {
//                 auto& sell = data["sell"];
//                 if (sell.isNumeric())
//                 {
//                     auto k = "";
//                     auto v = sell.asUInt();
//                     sells.insert({ k, v });
//                 }
//                 else if (sell.isObject())
//                 {
//                     for (auto i = sell.begin(); i != sell.end(); i++)
//                     {
//                         auto k = CP949(i.key().asString(), PLATFORM::Windows);
//                         auto v = (*i).asUInt();

//                         sells.insert({ k, v });
//                     }
//                 }
//                 else
//                 {
//                     throw std::runtime_error("invalid sell data type");
//                 }
//             }
//             for (auto& [k, v] : sells)
//             {
//                 if (fb::game::old_model::sell.contains(v) == false)
//                 {
//                     auto sstream = std::stringstream();
//                     sstream << v << " : sell에 정의되지 않았습니다.";
//                     throw std::runtime_error(sstream.str());
//                 }
//             }

//             if (data.isMember("buy"))
//             {
//                 if (data["buy"].isNumeric())
//                 {
//                     buy = data["buy"].asInt();
//                 }
//                 else if (data["buy"].isNull())
//                 {
//                 }
//                 else
//                 {
//                     throw std::runtime_error("invalid buy data type");
//                 }
//             }
            
//             if (buy.has_value() && fb::game::old_model::buy.contains(buy.value()) == false)
//             {
//                 auto sstream = std::stringstream();
//                 sstream << buy.value() << " : buy에 정의되지 않았습니다.";
//                 throw std::runtime_error(sstream.str());
//             }

//             assert_script(script, "function\\s+(on_interact)\\(\\w+,\\s*\\w+\\)", "on_interact", [](const auto& script) 
//             {
// #if defined DEBUG | defined _DEBUG
//                 auto out = std::ofstream(script, std::ios_base::app);
//                 out << "\n\nfunction on_interact(me, npc)\n"
//                     << "\n"
//                     << "end";
// #endif
//             });
//             lua::load(script);

//             {
//                 //name, look, color, script
//                 auto _ = std::lock_guard(*mutex);
//                 auto npc = new fb::model::npc(fb::model::npc::config 
//                     {
//                         {
//                             .name = name,
//                             .look = look,
//                             .color = color,
//                         },
//                         script,
//                         sells,
//                         buy,
//                         repair,
//                         hold_money,
//                         hold_item,
//                         rename
//                     });
//                 this->insert({id, std::unique_ptr<fb::model::npc>(npc)});
//                 callback(name, percentage);
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
//             error(name, e);
//         }
//         );

//     complete(count);
//     return true;
// }

// bool container::npc::load_spawn(const std::string& path, fb::game::context& context, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex           = std::make_unique<std::mutex>();
//     auto&                       config          = fb::config::get();
//     auto                        current_host    = config["id"].asString();
//     auto                        count           = fb::table::load
//     (
//         path, 
//         [&](Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                map_id          = std::stoi(key.asString());
//             auto                map             = fb::game::old_model::maps[map_id];
//             if (map == nullptr)
//                 return;

//             if (map->active == false)
//                 return;

//             for (auto& spawn : data)
//             {
//                 auto            npc_id          = spawn["npc"].asInt();
//                 auto            core            = fb::game::old_model::npcs[npc_id];
//                 if (core == nullptr)
//                     continue;

//                 point16_t       position(spawn["position"]["x"].asInt(), spawn["position"]["y"].asInt());
//                 auto            direction_str   = CP949(spawn["direction"].asString(), PLATFORM::Windows);
//                 auto            direction       = DIRECTION::BOTTOM;
//                 if (direction_str == "top")
//                     direction = DIRECTION::TOP;
//                 else if (direction_str == "right")
//                     direction = DIRECTION::RIGHT;
//                 else if (direction_str == "bottom")
//                     direction = DIRECTION::BOTTOM;
//                 else if (direction_str == "left")
//                     direction = DIRECTION::LEFT;
//                 else
//                     throw std::runtime_error(fb::model::const_value::string::MESSAGE_ASSET_INVALID_NPC_DIRECTION);

//                 auto                cloned      = core->make<fb::game::npc>(context);
//                 cloned->direction(direction);
                
//                 {
//                     auto _ = std::lock_guard(*mutex);
//                     cloned->map(map, position);
//                     callback(map->name(), percentage);
//                 }
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             auto                map_id = std::stoi(key.asString());
//             auto                map    = fb::game::old_model::maps[map_id];
//             error(map->name(), e);
//         }
//     );

//     complete(count);
//     return true;
// }

// MOB_SIZE container::mob::to_size(const std::string& size)
// {
//     if(size == "small")
//         return MOB_SIZE::SMALL;

//     if(size == "large")
//         return MOB_SIZE::LARGE;

//     throw std::runtime_error(fb::model::const_value::string::MESSAGE_ASSET_INVALID_MOB_SIZE);
// }

// MOB_ATTACK_TYPE container::mob::to_offensive(const std::string& offensive)
// {
//     if(offensive == "containment")
//         return MOB_ATTACK_TYPE::CONTAINMENT;

//     if(offensive == "counter")
//         return MOB_ATTACK_TYPE::COUNTER;

//     if(offensive == "none")
//         return MOB_ATTACK_TYPE::NONE;

//     if(offensive == "non move")
//         return MOB_ATTACK_TYPE::NON_MOVE;

//     if(offensive == "run away")
//         return MOB_ATTACK_TYPE::RUN_AWAY;

//     throw std::runtime_error(fb::model::const_value::string::MESSAGE_ASSET_INVALID_MOB_OFFENSIVE);
// }

// bool container::mob::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex = std::make_unique<std::mutex>();
//     auto                        count = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             uint16_t            id = std::stoi(key.asString());
//             auto                object_config = fb::model::object::config
//             {
//                 .name  = CP949(data["name"].asString(), PLATFORM::Windows),
//                 .look  = (uint16_t)(data["look"].asInt() + 0x7FFF),
//                 .color = (uint8_t)data["color"].asInt()
//             };

//             auto                life_config = fb::game::life::model::config
//             {
//                 object_config,
//                 /* defensive */ fb::game::defensive(data["defensive"]["physical"].asInt(), data["defensive"]["magical"].asInt()),
//                 /* hp        */ (uint32_t)data["hp"].asInt(),
//                 /* mp        */ (uint32_t)data["mp"].asInt(),
//                 /* exp       */ (uint32_t)data["experience"].asInt()
//             };

//             auto                config = fb::model::mob::config
//             {
//                 life_config,
//                 /* damage        */ fb::game::mob::damage(data["damage"]["min"].asInt(), data["damage"]["max"].asInt()),
//                 /* offensive     */ this->to_offensive(CP949(data["offensive"].asString(), PLATFORM::Windows)),
//                 /* size          */ this->to_size(CP949(data["size"].asString(), PLATFORM::Windows)),
//                 /* speed         */ std::chrono::milliseconds(data["speed"].asInt()),
//                 /* script_attack */ CP949(data["script"]["attack"].asString(), PLATFORM::Windows),
//                 /* script_die    */ CP949(data["script"]["die"].asString(), PLATFORM::Windows)
//             };

//             assert_script(config.script_attack, "function\\s+(on_attack)\\(\\w+,\\s*\\w+\\)", "on_attack", [](const auto& script) 
//             {
// #if defined DEBUG | defined _DEBUG
//                 auto out = std::ofstream(script, std::ios_base::app);
//                 out << "\n\nfunction on_attack(me, you)\n"
//                     << "    return false\n"
//                     << "end";
// #endif
//             });
//             lua::load(config.script_attack);

//             assert_script(config.script_die, "function\\s+(on_die)\\(\\w+,\\s*\\w+\\)", "on_die", [](const auto& script) 
//             {
// #if defined DEBUG | defined _DEBUG
//                 auto out = std::ofstream(script, std::ios_base::app);
//                 out << "\n\nfunction on_die(me, you)\n"
//                     << "\n"
//                     << "end";
// #endif
//             });
//             lua::load(config.script_die);

//             auto                mob = std::make_unique<fb::model::mob>(config);
//             {
//                 auto _ = std::lock_guard(*mutex);
//                 this->insert({id, std::move(mob)});
//                 callback(config.name, percentage);
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             auto                name = CP949(data["name"].asString(), PLATFORM::Windows);
//             error(name, e);
//         }
//     );

//     complete(count);
//     return true;
// }

// bool container::mob::load_drops(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex       = std::make_unique<std::mutex>();
//     char                        buffer[256] = {0,};
//     auto                        count       = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                name        = CP949(key.asString(), PLATFORM::Windows);
//             auto                core        = fb::game::old_model::mobs.name2mob(name);
//             if (core == nullptr)
//                 throw std::runtime_error(fb::format("%s (%s)", fb::model::const_value::string::MESSAGE_ASSET_INVALID_MOB_NAME, name.c_str()));

//             for(auto& x : data)
//             {
//                 float       percentage      = x["percentage"].asFloat();
//                 auto        item_name       = CP949(x["item"].asString(), PLATFORM::Windows);
//                 auto        item_core       = fb::game::old_model::items.name2item(item_name);

//                 if (item_core == nullptr)
//                     throw std::runtime_error(fb::format("%s (%s)", fb::model::const_value::string::MESSAGE_ASSET_INVALID_ITEM_NAME, item_name.c_str()));

//                 {
//                     auto _ = std::lock_guard(*mutex);
//                     core->push_drop(fb::game::mob::drop(item_core, percentage));
//                 }
//             }

//             {
//                 auto _ = std::lock_guard(*mutex);
//                 callback(name, percentage);
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             auto                name = CP949(key.asString(), PLATFORM::Windows);
//             error("", e);
//         }
//     );

//     complete(count);
//     return true;
// }

// bool container::mob::load_spawn(const std::string& path, fb::game::context& context, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex   = std::make_unique<std::mutex>();
//     auto                        count   = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                map_id  = std::stoi(key.asString());
//             auto                map     = fb::game::old_model::maps[map_id];
//             if (map == nullptr)
//                 return;

//             if (map->active == false)
//                 return;

//             for (auto& spawn : data)
//             {
//                 auto            mob_id  = spawn["mob"].asInt();
//                 auto            core    = fb::game::old_model::mobs[mob_id];
//                 if (core == nullptr)
//                     continue;

//                 uint16_t        x0      = spawn["area"]["x0"].asInt();
//                 uint16_t        x1      = spawn["area"]["x1"].asInt();
//                 uint16_t        y0      = spawn["area"]["y0"].asInt();
//                 uint16_t        y1      = spawn["area"]["y1"].asInt();
//                 uint16_t        count   = spawn["count"].asInt();
//                 uint32_t        rezen   = spawn["rezen time"].asInt();

//                 {
//                     auto _ = std::lock_guard(*mutex);
//                     for (int i = 0; i < count; i++)
//                     {
//                         auto        mob = core->make<fb::game::mob>(context, fb::game::mob::config { .alive = false });
//                         mob->spawn_point(x0, y0);
//                         mob->spawn_size(x1, y1);
//                         mob->respawn_time(std::chrono::seconds(rezen));
//                         mob->map(map);
//                     }
//                 }
//             }

//             {
//                 auto _ = std::lock_guard(*mutex);
//                 callback(map->name(), percentage);
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             auto                map_id  = std::stoi(key.asString());
//             auto                map     = fb::game::old_model::maps[map_id];
//             error(map->name(), e);
//         }
//     );

//     complete(count);
//     return true;
// }

// bool container::spell::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        mutex = std::make_unique<std::mutex>();
//     auto                        count = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             uint16_t            id   = std::stoi(key.asString());
//             const auto          name = CP949(data["name"].asString(), PLATFORM::Windows);
//             uint8_t             type = data["type"].asInt();

//             std::string         cast;
//             if (data.isMember("cast"))
//             {
//                 cast = CP949(data["cast"].asString(), PLATFORM::Windows);
//                 assert_script(cast, "function\\s+(on_cast)\\(\\w+,\\s*.*\\)", "on_cast", [](const auto& script) 
//                 {
// #if defined DEBUG | defined _DEBUG
//                     auto out = std::ofstream(script, std::ios_base::app);
//                     out << "\n\nfunction on_cast(me, spell)\n"
//                         << "\n"
//                         << "end";
// #endif
//                 });
//                 if(std::filesystem::exists(cast) == false)
//                     cast = "";
                
//                 lua::load(cast);
//             }

//             std::string         uncast;
//             if (data.isMember("uncast"))
//             {
//                 uncast = CP949(data["uncast"].asString(), PLATFORM::Windows);
//                 assert_script(uncast, "function\\s+(on_uncast)\\(\\w+,\\s*\\w+\\)", "on_uncast", [](const auto& script) 
//                 {
// #if defined DEBUG | defined _DEBUG
//                     auto out = std::ofstream(script, std::ios_base::app);
//                     out << "\n\nfunction on_uncast(me, spell)\n"
//                         << "\n"
//                         << "end";
// #endif
//                 });
//                 if(std::filesystem::exists(uncast) == false)
//                     uncast = "";
                
//                 lua::load(uncast);
//             }

//             std::string         concast;
//             if (data.isMember("concast"))
//             {
//                 concast = CP949(data["concast"].asString(), PLATFORM::Windows);
//                 assert_script(concast, "function\\s+(on_concast)\\(\\w+,\\s*\\w+\\)", "on_concast", [](const auto& script) 
//                 {
// #if defined DEBUG | defined _DEBUG
//                     auto out = std::ofstream(script, std::ios_base::app);
//                     out << "\n\nfunction on_concast(me, spell)\n"
//                         << "\n"
//                         << "end";
// #endif
//                 });

//                 lua::load(concast);
//             }

//             std::string         message;
//             if (data.isMember("message"))
//             {
//                 message = CP949(data["message"].asString(), PLATFORM::Windows);
//             }

//             {
//                 auto _ = std::lock_guard(*mutex);
//                 auto spell = new fb::model::spell(id, SPELL_TYPE(type), name, cast, uncast, concast, message);
//                 this->insert({id, std::unique_ptr<fb::model::spell>(spell)});
//                 callback(name, percentage);
//             }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto _ = std::lock_guard(*mutex);
//             const auto          name = CP949(data["name"].asString(), PLATFORM::Windows);
//             error(name, e);
//         }
//     );

//     complete(count);
//     return true;
// }

// bool container::cls::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        count = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             uint8_t             id      = key.asInt();      // class key
//             auto                classes = new class_data();

//             auto&               levels  = data["levels"];
//             for (auto i2 = levels.begin(); i2 != levels.end(); i2++)
//             {
//                 uint32_t        key       = i2.key().asInt();
//                 auto&           data      = *i2;
//                 classes->push(fb::game::ability(data["strength"].asInt(),
//                                                 data["intelligence"].asInt(),
//                                                 data["dexteritry"].asInt(),
//                                                 data["hp"].asInt(),
//                                                 data["mp"].asInt(),
//                                                 (uint32_t)data["exp"].asInt64()));
//             }

//             for (auto& promotion : data["promotions"])
//             {
//                 classes->push(CP949(promotion.asString(), PLATFORM::Windows));
//             }


//             this->push_back(std::unique_ptr<fb::game::class_data>(classes));
//             callback(std::to_string(id), percentage);
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             uint8_t             id = key.asInt();
//             error(std::to_string(id), e);
//         },
//         false
//     );

//     complete(count);
//     return true;
// }

// bool container::mix::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        count   = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                id      = key.asInt();
//             auto                itemmix = new fb::game::itemmix((float)data["percentage"].asDouble());

//             for(auto& require : data["require"])
//             {
//                 auto            item    = fb::game::old_model::items.name2item(CP949(require["item"].asString(), PLATFORM::Windows));
//                 uint32_t        count   = require["count"].asInt();
//                 itemmix->require_add(item, count);
//             }

//             for(auto& success: data["success"])
//             {
//                 auto            item    = fb::game::old_model::items.name2item(CP949(success["item"].asString(), PLATFORM::Windows));
//                 uint32_t        count   = success["count"].asInt();
//                 itemmix->success_add(item, count);
//             }

//             for(auto& failed: data["failed"])
//             {
//                 auto            item    = fb::game::old_model::items.name2item(CP949(failed["item"].asString(), PLATFORM::Windows));
//                 uint32_t        count   = failed["count"].asInt();
//                 itemmix->failed_add(item, count);
//             }

//             this->push_back(std::unique_ptr<fb::game::itemmix>(itemmix));
//             //callback(std::to_string(key), percentage);
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             //auto            key = key.asInt();
//             //error(std::to_string(key), e);
//         },
//         false
//     );

//     complete(count);
//     return true;
// }

// fb::game::itemmix* container::mix::find(const std::vector<fb::game::item*>& items)
// {
//     auto i = std::find_if(this->begin(), this->end(), 
//         [&items](std::unique_ptr<itemmix>& mx)
//         {
//             if(mx->require.size() != items.size())
//                 return false;

//             return mx->matched(items);
//         });

//     return i != this->end() ? i->get() : nullptr;
// }

// bool container::door::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        count   = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                id      = key.asInt();
//             auto                created = new fb::game::door::model();
//             for(auto& e : data)
//             {
//                 auto            open    = e["open"].asInt();
//                 auto            close   = e["close"].asInt();
//                 created->push_back(fb::game::door::element(open, close));
//             }

//             this->push_back(std::unique_ptr<fb::game::door::model>(created));
//             callback(std::to_string(id), percentage);
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto                id      = key.asInt();
//             error(std::to_string(id), e);
//         },
//         false
//     );

//     complete(count);
//     return true;
// }

// bool container::worlds::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        count   = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                name    = CP949(data["name"].asString(), PLATFORM::Windows);
//             auto                world   = new fb::game::wm::world(name);
//             for(auto& js_group : data["world"])
//             {
//                 auto            group   = new fb::game::wm::group();
//                 for(auto& js_offset : js_group)
//                 {
//                     auto        id      = CP949(js_offset["id"].asString(), PLATFORM::Windows);

//                     group->push(new fb::game::wm::offset
//                     (
//                         id, 
//                         CP949(js_offset["name"].asString(), PLATFORM::Windows),
//                         point16_t
//                         (
//                             js_offset["position"]["x"].asInt(), 
//                             js_offset["position"]["y"].asInt()
//                         ), 
//                         fb::game::wm::destination
//                         (
//                             point16_t
//                             (
//                                 js_offset["destination"]["x"].asInt(), 
//                                 js_offset["destination"]["y"].asInt()
//                             ), 
//                             old_model::maps[js_offset["destination"]["map"].asInt()]
//                         )
//                     ));
//                 }

//                 world->push(group);
//             }

//             this->push_back(std::unique_ptr<fb::game::wm::world>(world));
//             callback(name, percentage);
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             error(key.asString(), e);
//         },
//         false
//     );

//     complete(count);
//     return true;
// }

// int container::worlds::find(const std::string& id) const
// {
//     for(int i = 0; i < this->size(); i++)
//     {
//         auto& world = this->at(i);

//         const auto found = std::find_if
//         (
//             world->cbegin(), world->cend(),
//             [&] (const auto& group)
//             {
//                 return std::find_if
//                 (
//                     group->cbegin(), group->cend(),
//                     [&] (const auto& x)
//                     {
//                         return x->id == id;
//                     }
//                 ) != group->end();
//             }
//         );

//         if(found != world->cend())
//             return i;
//     }

//     return -1;
// }

// bool container::board::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        count   = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//             auto                id      = std::stoi(key.asCString());
//             auto                name    = CP949(data["name"].asString(), PLATFORM::Windows);
//             auto&               level   = data["level"];
//             auto                admin   = data["admin"].asBool();

//             auto                min_level = level.isNull() ? std::optional<uint32_t>() : (level["min"].isNull() ? std::optional<uint32_t>() : level["min"].asUInt());
//             auto                max_level = level.isNull() ? std::optional<uint32_t>() : (level["max"].isNull() ? std::optional<uint32_t>() : level["max"].asUInt());
//             this->insert({id, std::make_unique<fb::game::board::section>(id, name, min_level, max_level, admin) });
//             callback(std::to_string(id), percentage);
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto                id      = key.asInt();
//             error(std::to_string(id), e);
//         },
//         false
//     );

//     complete(count);
//     return true;
// }

// bool container::pursuit::load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete)
// {
//     auto                        count   = fb::table::load
//     (
//         path, 
//         [&] (Json::Value& key, Json::Value& data, double percentage)
//         {
//              auto                id      = std::stoi(key.asCString());
//              auto                dset    = pursuit_pair();
//              if (data.isObject())
//              {
//                  for(auto i = data.begin(); i != data.end(); i++)
//                  {
//                      auto k = CP949(i.key().asString(), PLATFORM::Windows);
//                      auto item = fb::game::old_model::items.name2item(k);
//                      if (item == nullptr)
//                      {
//                          auto sstream = std::stringstream();
//                          sstream << "invalid item name (" << k << ")";
//                          throw std::runtime_error(sstream.str());
//                      }

//                      auto price = std::optional<uint32_t>();
//                      if((*i).isNull())
//                      {
//                      }
//                      else if((*i).isNumeric())
//                      {
//                          price = (*i).asUInt();
//                      }
//                      else
//                      {
//                          auto sstream = std::stringstream();
//                          sstream << "invalid price type (" << k << ")";
//                          throw std::runtime_error(sstream.str());
//                      }

//                      dset.push_back({ item, price });
//                  }

//                  base_pursuit::insert({id, dset});
//                  callback(std::to_string(id), percentage);
//              }
//              else if (data.isArray())
//              {
//                  for (auto& e : data)
//                  {
//                      auto k = CP949(e.asString(), PLATFORM::Windows);
//                      auto item = fb::game::old_model::items.name2item(k);
//                      if (item == nullptr)
//                      {
//                          auto sstream = std::stringstream();
//                          sstream << "invalid item name (" << k << ")";
//                          throw std::runtime_error(sstream.str());
//                      }

//                      auto price = std::optional<uint32_t>{};
//                      dset.push_back({ item, price });
//                  }
//                  base_pursuit::insert({ id, dset });
//                  callback(std::to_string(id), percentage);
//              }
//              else
//              {
//                  throw std::runtime_error("invalid pursuit type");
//              }
//         },
//         [&] (Json::Value& key, Json::Value& data, const std::string& e)
//         {
//             auto                id      = key.asInt();
//             error(std::to_string(id), e);
//         },
//         false
//     );

//     complete(count);
//     return true;
// }

// container::pursuit_pair* container::pursuit::operator [] (uint16_t index)
// {
//     if(base_pursuit::contains(index) == false)
//         return nullptr;

//     return &base_pursuit::operator[](index);
// }