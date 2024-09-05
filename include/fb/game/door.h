#ifndef __DOOR_H__
#define __DOOR_H__

#include <vector>
#include <fb/game/mmo.h>
#include <fb/game/lua.h>
#include <fb/game/model.h>

using namespace fb::model;

namespace fb { namespace game {

class map;
class session;

class door : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    class model;

private:
    bool                        _opened = false;
    bool                        _locked = false;

public:
    const fb::game::map&        map;
    const fb::model::door&      model;
    const point16_t             position;
    const uint16_t              width;

public:
    door(const fb::game::map& map, const fb::model::door& model, const point16_t& position, bool opened);
    door(const door&) = delete;
    ~door();

public:
    //const door::model&          based() const;
    bool                        toggle();
    bool                        opened() const;
    bool                        locked() const;
    void                        lock(bool value);

public:
    static int                  builtin_toggle(lua_State* lua);
    static int                  builtin_locked(lua_State* lua);
    static int                  builtin_lock(lua_State* lua);
    static int                  builtin_opened(lua_State* lua);
    static int                  builtin_update(lua_State* lua);
};

class doors : private std::unordered_map<uint64_t, std::unique_ptr<door>>
{
public:
    class iterator;
    class const_iterator;

    friend class iterator;
    friend class const_iterator;

public:
    const fb::game::map&        map;

public:
    using unordered_map<uint64_t, std::unique_ptr<door>>::begin;
    using unordered_map<uint64_t, std::unique_ptr<door>>::end;
    using unordered_map<uint64_t, std::unique_ptr<door>>::cbegin;
    using unordered_map<uint64_t, std::unique_ptr<door>>::cend;
    using unordered_map<uint64_t, std::unique_ptr<door>>::size;

public:
    doors(const fb::game::map& map);
    doors(const doors&) = delete;
    ~doors();

public:
    iterator                    begin();
    iterator                    end();
    const_iterator              begin() const;
    const_iterator              end() const;
    void                        add(const point16_t& position, const fb::model::door& model, bool opened);
    door*                       find(const point16_t position) const;
    door*                       find(const session& session) const;
};

class doors::iterator : public std::unordered_map<uint64_t, std::unique_ptr<door>>::iterator
{
public:
    std::optional<std::pair<point16_t, door&>> pair;

public:
    iterator(const std::unordered_map<uint64_t, std::unique_ptr<door>>::iterator& i, const doors& container);
    ~iterator() = default;

public:
    std::pair<point16_t, door&> operator * ();
};

class doors::const_iterator : public std::unordered_map<uint64_t, std::unique_ptr<door>>::const_iterator
{
public:
    const std::optional<std::pair<point16_t, door&>> pair;

public:
    const_iterator(const std::unordered_map<uint64_t, std::unique_ptr<door>>::const_iterator& i, const doors& container);
    ~const_iterator() = default;

public:
    const std::pair<point16_t, door&> operator * () const;
};

} }

#endif // !__DOOR_H__