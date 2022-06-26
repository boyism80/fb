#ifndef __GROUP_H__
#define __GROUP_H__

#include <fb/game/session.h>
// #include <fb/game/lua.h>

namespace fb { namespace game {

class session;

class group : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

private:
    session*                        _leader;
    session::container              _members;

private:
    group(session& leader);

public:
    ~group();

public:
    session*                        enter(session& session);
    session*                        leave(session& session);
    bool                            contains(session& session);
    session&                        leader() const;
    const session::container&       members() const;

public:
    static fb::game::group*         create(session& leader);
    static void                     destroy(fb::game::group& group);

#pragma region built-in methods
public:
    static int                      builtin_members(lua_State* lua);
    static int                      builtin_leader(lua_State* lua);
#pragma endregion
};

} }

#endif // !__GROUP_H__
