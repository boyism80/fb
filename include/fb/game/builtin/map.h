#ifndef FB_GAME_BUILTIN_MAP_H
#define FB_GAME_BUILTIN_MAP_H

#include <fb/game/map.h>

namespace fb::game::builtin {

struct map
{
    /**
     * @brief      Lua binding for getting the map's model data.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding for getting the map's width.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_width(lua_State* L);

    /**
     * @brief      Lua binding for getting the map's height.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_height(lua_State* L);

    /**
     * @brief      Lua binding for getting the map's total area.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_area(lua_State* L);

    /**
     * @brief      Lua binding for getting all objects on the map.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_objects(lua_State* L);

    /**
     * @brief      Lua binding for finding objects near a specific position.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_nears(lua_State* L);

    /**
     * @brief      Lua binding for checking if movement is possible.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_movable(lua_State* L);

    /**
     * @brief      Lua binding for getting a specific door on the map.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_door(lua_State* L);

    /**
     * @brief      Lua binding for getting all doors on the map.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_doors(lua_State* L);

    /**
     * @brief      Lua binding for checking if map contains specified object.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_contains(lua_State* L);

    /**
     * @brief      Lua binding for finding objects at or below a position.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_belows(lua_State* L);

    /**
     * @brief      Lua binding for getting tile information at coordinates.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_tile(lua_State* L);

    /**
     * @brief      Lua binding for getting objects at a specific position.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_at(lua_State* L);

    /**
     * @brief      Lua binding for updating multiple objects on the map.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_bulk_update(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_MAP_H
