#ifndef FB_GAME_BUILTIN_OBJECT_H
#define FB_GAME_BUILTIN_OBJECT_H

#include <fb/game/object.h>

namespace fb::game::builtin {

struct object
{
    static int builtin_model(lua_State* L);
    static int builtin_oid(lua_State* L);
    static int builtin_eq(lua_State* L);
    static int builtin_tostring(lua_State* L);
    static int builtin_destroy(lua_State* L);
    static int builtin_name(lua_State* L);
    static int builtin_sound(lua_State* L);
    static int builtin_position(lua_State* L);
    static int builtin_front_position(lua_State* L);
    static int builtin_direction(lua_State* L);
    static int builtin_chat(lua_State* L);
    static int builtin_buff(lua_State* L);
    static int builtin_unbuff(lua_State* L);
    static int builtin_isbuff(lua_State* L);
    static int builtin_effect(lua_State* L);
    static int builtin_map(lua_State* L);
    static int builtin_mkitem(lua_State* L);
    static int builtin_sight_in(lua_State* L);
    static int builtin_nears(lua_State* L);
    static int builtin_front(lua_State* L);
    static int builtin_is(lua_State* L);
    static int builtin_thread(lua_State* L);
    static int builtin_near(lua_State* L);
    static int builtin_buffs(lua_State* L);
    static int builtin_hidden(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_OBJECT_H