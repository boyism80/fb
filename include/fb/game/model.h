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

template <typename K, typename V>
class model_map : private std::map<K, std::unique_ptr<V>>
{
protected:
    model_map() {}
    virtual ~model_map() {}

public:
    using std::map<K, std::unique_ptr<V>>::begin;
    using std::map<K, std::unique_ptr<V>>::end;
    using std::map<K, std::unique_ptr<V>>::cbegin;
    using std::map<K, std::unique_ptr<V>>::cend;
    using std::map<K, std::unique_ptr<V>>::size;
    using std::map<K, std::unique_ptr<V>>::contains;

protected:
    using std::map<K, std::unique_ptr<V>>::insert;

public:
    V* operator [] (K k) const
    {
        auto found = this->find(k);
        if (found == this->end())
            return nullptr;

        return found->second.get();
    }
};

template <typename V>
class model_vector : private std::vector<std::unique_ptr<V>>
{
public:
    using std::vector<std::unique_ptr<V>>::begin;
    using std::vector<std::unique_ptr<V>>::end;
    using std::vector<std::unique_ptr<V>>::cbegin;
    using std::vector<std::unique_ptr<V>>::cend;
    using std::vector<std::unique_ptr<V>>::size;
    using std::vector<std::unique_ptr<V>>::at;

protected:
    using std::vector<std::unique_ptr<V>>::push_back;

protected:
    model_vector() = default;
    virtual ~model_vector() = default;

public:
    V* operator [] (int index)
    {
        if (index > this->size() - 1)
            return nullptr;

        return std::vector<std::unique_ptr<V>>::operator[](index).get();
    }
};

namespace container {

class map : public model_map<uint16_t, fb::game::map>
{
public:
    friend class fb::game::table;

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
};

class item : public model_map<uint16_t, fb::game::item::model>
{
private:
    friend class fb::game::table;

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
};

class npc : public model_map<uint16_t, fb::game::npc::model>
{
private:
    friend class fb::game::table;

public:
    npc() = default;
    ~npc() = default;

public:
    bool                                   load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                   load_spawn(const std::string& path, fb::game::context& context, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::npc::model*                  name2npc(const std::string& name);
};

class mob : public model_map<uint16_t, fb::game::mob::model>
{
private:
    friend class fb::game::table;

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
};

class spell : public model_map<uint16_t, fb::game::spell>
{
private:
    friend class fb::game::table;

public:
    spell() = default;
    ~spell() = default;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::spell*                        name2spell(const std::string& name);
};

class cls : public model_vector<fb::game::class_data>
{
private:
    friend class fb::game::table;

public:
    cls() = default;
    ~cls() = default;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    const std::string*                      class2name(uint8_t cls, uint8_t promotion);
    bool                                    name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id);
    uint32_t                                exp(uint8_t class_id, uint8_t level);
};

class mix : public model_vector<fb::game::itemmix>
{
private:
    friend class fb::game::table;

public:
    mix() = default;
    ~mix() = default;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::itemmix*                      find(const std::vector<fb::game::item*>& items);
};

class door : public model_vector<fb::game::door::model>
{
private:
    friend class fb::game::table;

public:
    door() = default;
    ~door() = default;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
};

class worlds : public model_vector<fb::game::wm::world>
{
public:
    worlds() = default;
    ~worlds() = default;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    int                                     find(const std::string& id) const;
};

class board : public model_map<uint32_t, fb::game::board::section>
{

public:
    board() = default;
    ~board() = default;

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
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
