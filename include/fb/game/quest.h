#ifndef FB_GAME_QUEST_H
#define FB_GAME_QUEST_H

#include <cstdint>
#include <fb/model/model.h>
#include <fb/lua.h>

namespace fb::game {

class character;

class quest : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

private:
    uint32_t _step      = 0;
    uint32_t _progress  = 0;
    bool     _completed = false;

public:
    uint32_t   id;
    character& owner;

public:
    quest(uint32_t id, character& owner);
    quest(uint32_t id, character& owner, uint32_t step, uint32_t progress, bool completed);
    ~quest() = default;

public:
    uint32_t step() const;
    uint32_t progress() const;
    bool     inc_progress(uint32_t value = 1);
    bool     inc_step(uint32_t value = 1);
    bool     complete();
    bool     completed() const;

public:
    static int builtin_model(lua_State* L);
    static int builtin_step(lua_State* L);
    static int builtin_progress(lua_State* L);
    static int builtin_inc_progress(lua_State* L);
    static int builtin_inc_step(lua_State* L);
    static int builtin_complete(lua_State* L);
    static int builtin_completed(lua_State* L);
    static int builtin_completed_progress(lua_State* L);
    static int builtin_completed_step(lua_State* L);

public:
};

} // namespace fb::game

#endif // FB_GAME_QUEST_H