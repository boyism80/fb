#ifndef __MODEL_H__
#define __MODEL_H__

#include <map>
#include <queue>
#include <filesystem>
#include <fb/game/map.h>
#include <fb/game/board.h>
#include <fb/core/table.h>
#include <boost/xpressive/xpressive.hpp>

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
    map() = default;
    ~map() = default;

private:
    static fb::game::map::EFFECT            to_effect(const std::string& effect);
    static fb::game::map::OPTION            to_option(const Json::Value& data);
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
    item() = default;
    ~item() = default;

private:
    static fb::game::item::model*               create(uint32_t id, const Json::Value& data);
    static fb::game::item::conditions           to_condition(const Json::Value& data);
    static fb::game::item::DEATH_PENALTY_TYPE   to_penalty(const std::string& penalty);

public:
    bool                                   load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::item::model*                 name2item(const std::string& name);

public:
    fb::game::item::model*                 operator [] (uint16_t id);
};

using base_npc = std::map<uint16_t, std::unique_ptr<fb::game::npc::model>>;
class npc : private base_npc
{
private:
    friend class fb::game::table;

public:
    using base_npc::begin;
    using base_npc::end;
    using base_npc::cbegin;
    using base_npc::cend;
    using base_npc::size;

public:
    npc() = default;
    ~npc() = default;

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
    mob() = default;
    ~mob() = default;

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
    spell() = default;
    ~spell() = default;

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
    cls() = default;
    ~cls() = default;

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
    mix() = default;
    ~mix() = default;

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
    door() = default;
    ~door() = default;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::door::model*                  operator [] (int index);
};

class worlds : private std::vector<std::unique_ptr<fb::game::wm::world>>
{
public:
    worlds() = default;
    ~worlds() = default;

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

class regex
{
public:
    enum class TYPE
    {
        SELL = 0, BUY = 1
    };

private:
    using iterator = std::map<TYPE, boost::xpressive::sregex>::iterator;
    using const_iterator = std::map<TYPE, boost::xpressive::sregex>::const_iterator;

private:
    std::map<TYPE, boost::xpressive::sregex> _regexs;

public:
    regex() = default;
    ~regex() = default;

public:
    iterator                                begin();
    iterator                                end();
    const_iterator                          cbegin() const;
    const_iterator                          cend() const;
    size_t                                  size() const;
    bool                                    contains(TYPE k) const;
    boost::xpressive::sregex&               operator [] (TYPE k);

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
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
    static fb::game::container::pursuit     sell;
    static fb::game::container::pursuit     buy;
    static fb::game::container::regex       regex;

private:
    model();
    ~model();
};

} }

#endif // !__MODEL_H__
