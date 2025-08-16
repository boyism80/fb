#ifndef FB_GAME_BUILTIN_SERVER_H
#define FB_GAME_BUILTIN_SERVER_H

#include <fb/game/server.h>

namespace fb::game::builtin {

class server
{
public:
    /**
     * @brief      Lua builtin function to log a message.
     *
     * @param      L   The Lua state containing log message arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_log(lua_State* L);

    /**
     * @brief      Lua builtin function to seed the random number generator.
     *
     * @param      L   The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_seed(lua_State* L);

    /**
     * @brief      Lua builtin function to pause execution for a specified time.
     *
     * @param      L   The Lua state containing sleep duration argument.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_sleep(lua_State* L);

    /**
     * @brief      Lua builtin function to get current Baram calendar time.
     *             Baram calendar starts from 1996-03-31 00:00:00,
     *             where 1 Baram day equals 3 real hours.
     *
     * @param      L   The Lua state containing no arguments.
     *
     * @return     Number of return values pushed to Lua stack (Baram calendar time).
     */
    static int builtin_baram_time(lua_State* L);

    /**
     * @brief      Lua builtin function to find a mob by name.
     *
     * @param      L   The Lua state containing mob name argument.
     *
     * @return     Number of return values pushed to Lua stack (mob object or nil).
     */
    static int builtin_name2mob(lua_State* L);

    /**
     * @brief      Lua builtin function to find a spell by name.
     *
     * @param      L   The Lua state containing spell name argument.
     *
     * @return     Number of return values pushed to Lua stack (spell object or nil).
     */
    static int builtin_name2spell(lua_State* L);

    /**
     * @brief      Lua builtin function to find an NPC by name.
     *
     * @param      L   The Lua state containing NPC name argument.
     *
     * @return     Number of return values pushed to Lua stack (NPC object or nil).
     */
    static int builtin_name2npc(lua_State* L);

    /**
     * @brief      Lua builtin function to find a map by name.
     *
     * @param      L   The Lua state containing map name argument.
     *
     * @return     Number of return values pushed to Lua stack (map object or nil).
     */
    static int builtin_name2map(lua_State* L);

    /**
     * @brief      Lua builtin function to find a character by name.
     *
     * @param      L   The Lua state containing character name argument.
     *
     * @return     Number of return values pushed to Lua stack (character object or nil).
     */
    static int builtin_name2ch(lua_State* L);

    /**
     * @brief      Lua builtin function to find an item by name.
     *
     * @param      L   The Lua state containing item name argument.
     *
     * @return     Number of return values pushed to Lua stack (item object or nil).
     */
    static int builtin_name2item(lua_State* L);

    /**
     * @brief      Lua builtin function to handle pursuit sell operations.
     *
     * @param      L   The Lua state containing pursuit sell arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_pursuit_sell(lua_State* L);

    /**
     * @brief      Lua builtin function to get pursuit sell price information.
     *
     * @param      L   The Lua state containing price query arguments.
     *
     * @return     Number of return values pushed to Lua stack (price information).
     */
    static int builtin_pursuit_sell_price(lua_State* L);

    /**
     * @brief      Lua builtin function to get pursuit sell item name.
     *
     * @param      L   The Lua state containing item query arguments.
     *
     * @return     Number of return values pushed to Lua stack (item name).
     */
    static int builtin_pursuit_sell_name(lua_State* L);

    /**
     * @brief      Lua builtin function to handle pursuit buy operations.
     *
     * @param      L   The Lua state containing pursuit buy arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_pursuit_buy(lua_State* L);

    /**
     * @brief      Lua builtin function to create or manage timers.
     *
     * @param      L   The Lua state containing timer configuration arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_timer(lua_State* L);

    /**
     * @brief      Lua builtin function to get or set weather information.
     *
     * @param      L   The Lua state containing weather query/set arguments.
     *
     * @return     Number of return values pushed to Lua stack (weather data).
     */
    static int builtin_weather(lua_State* L);

    /**
     * @brief      Lua builtin function to get or set brightness/lighting.
     *
     * @param      L   The Lua state containing brightness arguments.
     *
     * @return     Number of return values pushed to Lua stack (brightness data).
     */
    static int builtin_bright(lua_State* L);

    /**
     * @brief      Lua builtin function to find objects with specific name patterns.
     *
     * @param      L   The Lua state containing name pattern arguments.
     *
     * @return     Number of return values pushed to Lua stack (matching objects).
     */
    static int builtin_name_with(lua_State* L);

    /**
     * @brief      Lua builtin function to validate Korean text input.
     *
     * @param      L   The Lua state containing text to validate.
     *
     * @return     Number of return values pushed to Lua stack (validation result).
     */
    static int builtin_assert_korean(lua_State* L);

    /**
     * @brief      Lua builtin function to handle CP949 encoding operations.
     *
     * @param      L   The Lua state containing encoding arguments.
     *
     * @return     Number of return values pushed to Lua stack (encoded text).
     */
    static int builtin_cp949(lua_State* L);

    /**
     * @brief      Lua builtin function to send broadcast messages.
     *
     * @param      L   The Lua state containing broadcast message arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_broadcast(lua_State* L);

    /**
     * @brief      Lua builtin function to assert that a character is alive.
     *
     * @param      L   The Lua state containing character to check.
     *
     * @return     Number of return values pushed to Lua stack (assertion result).
     */
    static int builtin_assert_alive(lua_State* L);

    /**
     * @brief      Lua builtin function to output debug information.
     *
     * @param      L   The Lua state containing debug message arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_debug(lua_State* L);

    /**
     * @brief      Lua builtin function to convert character name to class type.
     *
     * @param      L   The Lua state containing character name argument.
     *
     * @return     Number of return values pushed to Lua stack (class type).
     */
    static int builtin_name2class(lua_State* L);

    /**
     * @brief      Lua builtin function to convert class type to class name.
     *
     * @param      L   The Lua state containing class type argument.
     *
     * @return     Number of return values pushed to Lua stack (class name).
     */
    static int builtin_class2name(lua_State* L);

    /**
     * @brief      Lua builtin function to save character data.
     *
     * @param      L   The Lua state containing character to save.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_save(lua_State* L);

    /**
     * @brief      Lua builtin function to create an NPC instance.
     *
     * @param      L   The Lua state containing NPC creation arguments.
     *
     * @return     Number of return values pushed to Lua stack (created NPC).
     */
    static int builtin_mknpc(lua_State* L);

    /**
     * @brief      Lua builtin function to get available maps list.
     *
     * @param      L   The Lua state containing map query arguments.
     *
     * @return     Number of return values pushed to Lua stack (maps list).
     */
    static int builtin_maps(lua_State* L);

    /**
     * @brief      Lua builtin function to initiate server shutdown.
     *
     * @param      L   The Lua state containing shutdown parameters.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_shutdown(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_SERVER_H