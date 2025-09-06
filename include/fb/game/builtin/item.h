#ifndef FB_GAME_BUILTIN_ITEM_H
#define FB_GAME_BUILTIN_ITEM_H

#include <fb/game/item.h>

namespace fb::game::builtin {

struct item
{
    static int builtin_model(lua_State* L);
    static int builtin_count(lua_State* L);
    static int builtin_durability(lua_State* L);
    static int builtin_rename(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_ITEM_H
