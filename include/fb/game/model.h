#ifndef __MODEL_H__
#define __MODEL_H__

#include <map>
#include <queue>
#include <filesystem>
#include <fb/game/map.h>
#include <fb/game/board.h>
#include <fb/core/table.h>

namespace fb { namespace game {

class table;

namespace container {

class map : private std::map<uint16_t, std::unique_ptr<fb::game::map>>
{
public:
    friend class fb::game::table;

public:
    using std::map<uint16_t, std::unique_ptr<fb::game::map>>::begin;
    using std::map<uint16_t, std::unique_ptr<fb::game::map>>::end;
    using std::map<uint16_t, std::unique_ptr<fb::game::map>>::cbegin;
    using std::map<uint16_t, std::unique_ptr<fb::game::map>>::cend;
    using std::map<uint16_t, std::unique_ptr<fb::game::map>>::size;

public:
    map();
    ~map();

private:
    static fb::game::map::effects           to_effect(const std::string& effect);
    static fb::game::map::options           to_option(const Json::Value& data);
    static bool                             load_data(uint16_t id, std::vector<char>& buffer);
    static bool                             load_blocks(uint16_t id, Json::Value& buffer);

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                    load_warps(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::map*                          name2map(const std::string& name);
    //void                                    clear(const std::function<bool(const fb::game::map&)>& fn);

public:
    fb::game::map*                          operator [] (uint16_t id) const;
};

class item : private std::map<uint16_t, std::unique_ptr<fb::game::item::model>>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, std::unique_ptr<fb::game::item::model>>::begin;
    using std::map<uint16_t, std::unique_ptr<fb::game::item::model>>::end;
    using std::map<uint16_t, std::unique_ptr<fb::game::item::model>>::cbegin;
    using std::map<uint16_t, std::unique_ptr<fb::game::item::model>>::cend;
    using std::map<uint16_t, std::unique_ptr<fb::game::item::model>>::size;

public:
    item();
    ~item();

private:
    static fb::game::item::model*          create(uint32_t id, const Json::Value& data);
    static fb::game::item::conditions      to_condition(const Json::Value& data);
    static fb::game::item::penalties       to_penalty(const std::string& penalty);

public:
    bool                                   load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::item::model*                 name2item(const std::string& name);

public:
    fb::game::item::model*                 operator [] (uint16_t id);
};

class npc : private std::map<uint16_t, std::unique_ptr<fb::game::npc::model>>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, std::unique_ptr<fb::game::npc::model>>::begin;
    using std::map<uint16_t, std::unique_ptr<fb::game::npc::model>>::end;
    using std::map<uint16_t, std::unique_ptr<fb::game::npc::model>>::cbegin;
    using std::map<uint16_t, std::unique_ptr<fb::game::npc::model>>::cend;
    using std::map<uint16_t, std::unique_ptr<fb::game::npc::model>>::size;

public:
    npc();
    ~npc();

public:
    bool                                   load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                   load_spawn(const std::string& path, fb::game::context& context, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::npc::model*                  name2npc(const std::string& name);

public:
    fb::game::npc::model*                  operator [] (uint16_t id);
};

class mob : private std::map<uint16_t, std::unique_ptr<fb::game::mob::model>>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, std::unique_ptr<fb::game::mob::model>>::begin;
    using std::map<uint16_t, std::unique_ptr<fb::game::mob::model>>::end;
    using std::map<uint16_t, std::unique_ptr<fb::game::mob::model>>::cbegin;
    using std::map<uint16_t, std::unique_ptr<fb::game::mob::model>>::cend;
    using std::map<uint16_t, std::unique_ptr<fb::game::mob::model>>::size;

public:
    mob();
    ~mob();

private:
    static fb::game::mob::sizes            to_size(const std::string& size);
    static fb::game::mob::offensive_type   to_offensive(const std::string& offensive);

public:
    bool                                   load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                   load_drops(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                   load_spawn(const std::string& path, fb::game::context& context, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::mob::model*                  name2mob(const std::string& name);

public:
    fb::game::mob::model*                  operator [] (uint16_t id);
};

class spell : private std::map<uint16_t, std::unique_ptr<fb::game::spell>>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, std::unique_ptr<fb::game::spell>>::begin;
    using std::map<uint16_t, std::unique_ptr<fb::game::spell>>::end;
    using std::map<uint16_t, std::unique_ptr<fb::game::spell>>::cbegin;
    using std::map<uint16_t, std::unique_ptr<fb::game::spell>>::cend;
    using std::map<uint16_t, std::unique_ptr<fb::game::spell>>::size;

public:
    spell();
    ~spell();

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::spell*                        name2spell(const std::string& name);

public:
    fb::game::spell*                        operator [] (uint16_t id);
};

class cls : private std::vector<std::unique_ptr<fb::game::class_data>>
{
private:
    friend class fb::game::table;

public:
    using std::vector<std::unique_ptr<fb::game::class_data>>::begin;
    using std::vector<std::unique_ptr<fb::game::class_data>>::end;
    using std::vector<std::unique_ptr<fb::game::class_data>>::cbegin;
    using std::vector<std::unique_ptr<fb::game::class_data>>::cend;
    using std::vector<std::unique_ptr<fb::game::class_data>>::size;

public:
    cls();
    ~cls();

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    const std::string*                      class2name(uint8_t cls, uint8_t promotion);
    bool                                    name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id);
    uint32_t                                exp(uint8_t class_id, uint8_t level);

public:
    fb::game::class_data*                   operator [] (int index);
};

class mix : private std::vector<std::unique_ptr<fb::game::itemmix>>
{
private:
    friend class fb::game::table;

public:
    using std::vector<std::unique_ptr<fb::game::itemmix>>::begin;
    using std::vector<std::unique_ptr<fb::game::itemmix>>::end;
    using std::vector<std::unique_ptr<fb::game::itemmix>>::cbegin;
    using std::vector<std::unique_ptr<fb::game::itemmix>>::cend;
    using std::vector<std::unique_ptr<fb::game::itemmix>>::size;

public:
    mix();
    ~mix();

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::itemmix*                      find(const std::vector<fb::game::item*>& items);

public:
    fb::game::itemmix*                      operator [] (int index);
};

class door : private std::vector<std::unique_ptr<fb::game::door::model>>
{
private:
    friend class fb::game::table;

public:
    using std::vector<std::unique_ptr<fb::game::door::model>>::begin;
    using std::vector<std::unique_ptr<fb::game::door::model>>::end;
    using std::vector<std::unique_ptr<fb::game::door::model>>::cbegin;
    using std::vector<std::unique_ptr<fb::game::door::model>>::cend;
    using std::vector<std::unique_ptr<fb::game::door::model>>::size;

public:
    door();
    ~door();

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::door::model*                  operator [] (int index);
};

class worlds : private std::vector<std::unique_ptr<fb::game::wm::world>>
{
public:
    worlds();
    ~worlds();

public:
    using std::vector<std::unique_ptr<fb::game::wm::world>>::begin;
    using std::vector<std::unique_ptr<fb::game::wm::world>>::end;
    using std::vector<std::unique_ptr<fb::game::wm::world>>::cbegin;
    using std::vector<std::unique_ptr<fb::game::wm::world>>::cend;
    using std::vector<std::unique_ptr<fb::game::wm::world>>::size;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    int                                     find(const std::string& id) const;

public:
    fb::game::wm::world*                    operator [] (int index);
};

class board : private std::map<uint32_t, std::unique_ptr<fb::game::board::section>>
{

public:
    board() = default;
    ~board() = default;

public:
    using std::map<uint32_t, std::unique_ptr<fb::game::board::section>>::begin;
    using std::map<uint32_t, std::unique_ptr<fb::game::board::section>>::end;
    using std::map<uint32_t, std::unique_ptr<fb::game::board::section>>::cbegin;
    using std::map<uint32_t, std::unique_ptr<fb::game::board::section>>::cend;
    using std::map<uint32_t, std::unique_ptr<fb::game::board::section>>::size;
    using std::map<uint32_t, std::unique_ptr<fb::game::board::section>>::contains;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    fb::game::board::section*               operator [] (uint32_t index);
};


using pursuit_pair = std::vector<std::pair<fb::game::item::model*, std::optional<uint32_t>>>;
using base_pursuit = std::map<uint16_t, pursuit_pair>;
class pursuit : private base_pursuit
{
public:
    pursuit() = default;
    ~pursuit() = default;

public:
    using base_pursuit::begin;
    using base_pursuit::end;
    using base_pursuit::cbegin;
    using base_pursuit::cend;
    using base_pursuit::size;
    using base_pursuit::contains;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    pursuit_pair*                           operator [] (uint16_t index);
};

}

class session;

class model
{
public:
    static std::vector<clan*>               clans;
    static fb::game::container::map         maps;
    static fb::game::container::worlds      worlds;
    static fb::game::container::item        items;
    static fb::game::container::npc         npcs;
    static fb::game::container::mob         mobs;
    static fb::game::container::spell       spells;
    static fb::game::container::cls         classes;
    static fb::game::container::mix         mixes;
    static fb::game::container::door        doors;
    static fb::game::container::board       boards;
    static fb::game::container::pursuit     sale;
    static fb::game::container::pursuit     purchase;

private:
    model();
    ~model();
};

} }

#endif // !__MODEL_H__
