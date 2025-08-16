#ifndef FB_GAME_BUILTIN_OBJECT_H
#define FB_GAME_BUILTIN_OBJECT_H

#include <fb/game/object.h>

namespace fb::game::builtin {

struct object
{
    /**
     * @brief      Lua binding to get the object's model data.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding to get the object's unique ID.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_id(lua_State* L);

    /**
     * @brief      Lua binding for object equality comparison.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_eq(lua_State* L);

    /**
     * @brief      Lua binding to convert object to string representation.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_tostring(lua_State* L);

    /**
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_destroy(lua_State* L);

    /**
     * @brief      Lua binding to get the object's name.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_name(lua_State* L);

    /**
     * @brief      Lua binding to play a sound effect for the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_sound(lua_State* L);

    /**
     * @brief      Lua binding to get or set the object's position.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_position(lua_State* L);

    /**
     * @brief      Lua binding to get the position in front of the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_front_position(lua_State* L);

    /**
     * @brief      Lua binding to get or set the object's facing direction.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_direction(lua_State* L);

    /**
     * @brief      Lua binding to send a chat message from the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_chat(lua_State* L);

    /**
     * @brief      Lua binding to apply a buff to the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff(lua_State* L);

    /**
     * @brief      Lua binding to remove a buff from the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_unbuff(lua_State* L);

    /**
     * @brief      Lua binding to check if the object has a specific buff.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_isbuff(lua_State* L);

    /**
     * @brief      Lua binding to apply a visual effect to the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_effect(lua_State* L);

    /**
     * @brief      Lua binding to get the object's current map.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_map(lua_State* L);

    /**
     * @brief      Lua binding to create an item for the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mkitem(lua_State* L);

    /**
     * @brief      Lua binding to get objects within line of sight.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_sight_in(lua_State* L);

    /**
     * @brief      Lua binding to get nearby objects.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_nears(lua_State* L);

    /**
     * @brief      Lua binding to get objects in front of this object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_front(lua_State* L);

    /**
     * @brief      Lua binding to check if the object is of a specific type.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_is(lua_State* L);

    /**
     * @brief      Lua binding to get the object's thread.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_thread(lua_State* L);

    /**
     * @brief      Lua binding to get the nearest object of a specific type.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_near(lua_State* L);

    /**
     * @brief      Lua binding to get all buffs applied to the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buffs(lua_State* L);

    /**
     * @brief      Lua binding to check if the object is hidden.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_hidden(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_OBJECT_H