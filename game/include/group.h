#ifndef __GROUP_H__
#define __GROUP_H__

#include <character.h>
// #include <lua.h>

namespace fb { namespace game {

class character;

class group : public lua::luable
{
public:
    LUA_PROTOTYPE

private:
    character*                        _leader;
    character::container              _members;

private:
    group(character& leader);

public:
    ~group();

public:
    character*                        enter(character& session);
    character*                        leave(character& session);
    bool                            contains(character& session);
    character&                        leader() const;
    const character::container&       members() const;

public:
    static fb::game::group*         create(character& leader);
    static void                     destroy(fb::game::group& group);

public:
    static int                      builtin_members(lua_State* lua);
    static int                      builtin_leader(lua_State* lua);
};

} }

#endif // !__GROUP_H__
