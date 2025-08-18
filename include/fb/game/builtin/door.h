#ifndef FB_GAME_BUILTIN_DOOR_H
#define FB_GAME_BUILTIN_DOOR_H

#include <fb/game/door.h>

namespace fb::game::builtin {

struct door
{
    static int builtin_toggle(lua_State* L);
    static int builtin_locked(lua_State* L);
    static int builtin_lock(lua_State* L);
    static int builtin_opened(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_DOOR_H
