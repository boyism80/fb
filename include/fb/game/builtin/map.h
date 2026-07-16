#ifndef FB_GAME_BUILTIN_MAP_H
#define FB_GAME_BUILTIN_MAP_H

#include <fb/game/map.h>

namespace fb::game::builtin {

struct map
{
    static int builtin_model(lua_State* L);
    static int builtin_width(lua_State* L);
    static int builtin_height(lua_State* L);
    static int builtin_area(lua_State* L);
    static int builtin_objects(lua_State* L);
    static int builtin_nears(lua_State* L);
    static int builtin_movable(lua_State* L);
    static int builtin_door(lua_State* L);
    static int builtin_doors(lua_State* L);
    static int builtin_contains(lua_State* L);
    static int builtin_belows(lua_State* L);
    static int builtin_tile(lua_State* L);
    static int builtin_at(lua_State* L);
    static int builtin_block(lua_State* L);
    static int builtin_bulk_update(lua_State* L);
    static int builtin_instance(lua_State* L);
    static int builtin_slot(lua_State* L);
    static int builtin_clone(lua_State* L);
    static int builtin_destroy(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_MAP_H
