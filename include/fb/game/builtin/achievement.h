#ifndef FB_GAME_BUILTIN_ACHIEVEMENT_H
#define FB_GAME_BUILTIN_ACHIEVEMENT_H

#include <fb/game/achievement.h>

namespace fb::game::builtin {

struct achievement
{
    /**
     * @brief      Lua binding function to access the achievement model.
     *
     *             Provides Lua scripts with access to the underlying achievement model
     *             data, including configuration parameters, requirements, and rewards.
     *
     * @param      L     The Lua state for script execution.
     *
     * @return     The number of return values pushed to the Lua stack.
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding function to access the achievement text.
     *
     *             Provides Lua scripts with access to the achievement's text description,
     *             which may be custom text or the default description from the model.
     *
     * @param      L     The Lua state for script execution.
     *
     * @return     The number of return values pushed to the Lua stack.
     */
    static int builtin_text(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_ACHIEVEMENT_H
