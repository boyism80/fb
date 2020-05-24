#ifndef __GROUP_H__
#define __GROUP_H__

#include <vector>
#include "lua.h"

namespace fb { namespace game {

class session;

class group : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

private:
    session*                    _leader;
    std::vector<session*>       _members;

private:
    group(session& leader);
    ~group();

public:
    session*                    enter(session& session);
    session*                    leave(session& session);
    bool                        contains(session& session);
    session&                    leader() const;
    std::vector<session*>&      members();

public:
    static group*               create(session& leader);
    static void                 destroy(fb::game::group& group);

#pragma region built-in methods
public:
    static int                  builtin_members(lua_State* lua);
    static int                  builtin_leader(lua_State* lua);
#pragma endregion
};

} }

#endif // !__GROUP_H__
