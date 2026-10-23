#ifndef FB_GAME_BUILTIN_STATE_MACHINE_H
#define FB_GAME_BUILTIN_STATE_MACHINE_H

#include <fb/game/state_machine.h>

namespace fb::game::builtin {

struct state_machine
{
    static int builtin_name(lua_State* L);
    static int builtin_get(lua_State* L);
    static int builtin_set(lua_State* L);
    static int builtin_every(lua_State* L);
    static int builtin_after(lua_State* L);
    static int builtin_cancel(lua_State* L);
    static int builtin_call(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_STATE_MACHINE_H
