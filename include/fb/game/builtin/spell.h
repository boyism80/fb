#ifndef FB_GAME_BUILTIN_SPELL_H
#define FB_GAME_BUILTIN_SPELL_H

#include <fb/game/spell.h>

namespace fb::game::builtin {

struct spell
{
    /**
     * @brief      Lua binding for getting spell model data.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting spell delay.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_delay(lua_State* L);

    /**
     * @brief      Lua binding for setting spell delay with specific parameters.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_delay2(lua_State* L);
};

struct buff
{
    /**
     * @brief      Lua binding for getting buff model data.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting buff duration.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_time(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_SPELL_H
