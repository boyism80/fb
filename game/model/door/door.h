#ifndef __DOOR_H__
#define __DOOR_H__

#include <vector>
#include "model/mmo/mmo.h"
#include "model/lua/lua.h"

namespace fb { namespace game {

#pragma region forward declaration
class map;
class session;
#pragma endregion

class door : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region forward nested declaration
public:
    struct element;

public:
    class master;
#pragma endregion

#pragma region private field
private:
    fb::game::map*              _owner;
    master&                     _master;
    bool                        _opened;
    bool                        _locked;
#pragma endregion

#pragma region public field
public:
    const point16_t             position;
#pragma endregion

#pragma region constructor / destructor
public:
    door(fb::game::map* owner, door::master& master, const point16_t position, bool opened);
    ~door();
#pragma endregion

#pragma region public method
public:
    const door::master&         based() const;
    bool                        toggle();
    bool                        opened() const;
    bool                        locked() const;
    void                        lock(bool value);
    const fb::game::map&        map() const;
#pragma endregion

#pragma region built-in method
public:
    static int                  builtin_toggle(lua_State* lua);
    static int                  builtin_locked(lua_State* lua);
    static int                  builtin_lock(lua_State* lua);
    static int                  builtin_opened(lua_State* lua);
    static int                  builtin_update(lua_State* lua);
#pragma endregion
};

class doors : private std::vector<std::unique_ptr<door>>
{
#pragma region using
public:
    using std::vector<std::unique_ptr<door>>::begin;
    using std::vector<std::unique_ptr<door>>::end;
    using std::vector<std::unique_ptr<door>>::cbegin;
    using std::vector<std::unique_ptr<door>>::cend;
    using std::vector<std::unique_ptr<door>>::rbegin;
    using std::vector<std::unique_ptr<door>>::rend;
    using std::vector<std::unique_ptr<door>>::size;
#pragma endregion

#pragma region constructor / destructor
public:
    doors();
    ~doors();
#pragma endregion

#pragma region public method
public:
    void                        add(map* map, fb::game::door::master& master, const point16_t position, bool opened);
    door*                       find(const point16_t position);
    door*                       find(const session& session);
#pragma endregion
};

struct door::element
{
    uint16_t                    open;
    uint16_t                    close;

    element(uint16_t open, uint16_t close) : open(open), close(close) {}
    ~element() {}
};

class door::master : private std::vector<door::element>
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