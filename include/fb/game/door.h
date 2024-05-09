#ifndef __DOOR_H__
#define __DOOR_H__

#include <vector>
#include <fb/game/mmo.h>
#include <fb/game/lua.h>

namespace fb { namespace game {

class map;
class session;

class door : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct element;

public:
    class model;

private:
    model&                      _model;
    fb::game::map*              _owner  = nullptr;
    bool                        _opened = false;
    bool                        _locked = false;

public:
    const point16_t             position;

public:
    door(fb::game::map* owner, door::model& model, const point16_t position, bool opened);
    ~door();

public:
    const door::model&          based() const;
    bool                        toggle();
    bool                        opened() const;
    bool                        locked() const;
    void                        lock(bool value);
    const fb::game::map&        map() const;

public:
    static int                  builtin_toggle(lua_State* lua);
    static int                  builtin_locked(lua_State* lua);
    static int                  builtin_lock(lua_State* lua);
    static int                  builtin_opened(lua_State* lua);
    static int                  builtin_update(lua_State* lua);
};

class doors : private std::vector<std::unique_ptr<door>>
{
public:
    using std::vector<std::unique_ptr<door>>::begin;
    using std::vector<std::unique_ptr<door>>::end;
    using std::vector<std::unique_ptr<door>>::cbegin;
    using std::vector<std::unique_ptr<door>>::cend;
    using std::vector<std::unique_ptr<door>>::rbegin;
    using std::vector<std::unique_ptr<door>>::rend;
    using std::vector<std::unique_ptr<door>>::size;

public:
    doors();
    ~doors();

public:
    void                        add(map* map, fb::game::door::model& model, const point16_t position, bool opened);
    door*                       find(const point16_t position);
    door*                       find(const session& session);
};

struct door::element
{
    uint16_t                    open;
    uint16_t                    close;

    element(uint16_t open, uint16_t close) : open(open), close(close) { }
    ~element() { }
};

class door::model : private std::vector<door::element>
{
public:
    using std::vector<door::element>::begin;
    using std::vector<door::element>::end;
    using std::vector<door::element>::cbegin;
    using std::vector<door::element>::cend;
    using std::vector<door::element>::rbegin;
    using std::vector<door::element>::rend;
    using std::vector<door::element>::crbegin;
    using std::vector<door::element>::crend;
    using std::vector<door::element>::size;
    using std::vector<door::element>::push_back;
    using std::vector<door::element>::operator[];

private:
    bool                    matched(const fb::game::map& map, const point16_t& position, bool open) const;

public:
    bool                    find(const fb::game::map& map, point16_t& position, bool open) const;
};

} }

#endif // !__DOOR_H__