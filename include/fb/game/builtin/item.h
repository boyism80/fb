#ifndef FB_GAME_BUILTIN_ITEM_H
#define FB_GAME_BUILTIN_ITEM_H

#include <fb/game/item.h>

namespace fb::game::builtin {

struct item
{
    /**
     * @brief      Lua builtin function to get item model information.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua builtin function to get/set item count.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_count(lua_State* L);

    /**
     * @brief      Lua builtin function to get/set item durability.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_durability(lua_State* L);

    /**
     * @brief      Lua builtin function to rename an item.
     *
     * @param      L   The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_rename(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_ITEM_H
