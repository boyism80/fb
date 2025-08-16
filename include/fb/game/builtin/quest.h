#ifndef FB_GAME_BUILTIN_QUEST_H
#define FB_GAME_BUILTIN_QUEST_H

#include <fb/game/quest.h>

namespace fb::game::builtin {

struct quest
{
    static int builtin_model(lua_State* L);
    static int builtin_step(lua_State* L);
    static int builtin_progress(lua_State* L);
    static int builtin_param(lua_State* L);
    static int builtin_inc_progress(lua_State* L);
    static int builtin_inc_step(lua_State* L);
    static int builtin_complete(lua_State* L);
    static int builtin_completed(lua_State* L);
    static int builtin_completed_progress(lua_State* L);
    static int builtin_completed_step(lua_State* L);
};

} // namespace fb::game::builtin

#endif // !FB_GAME_BUILTIN_QUEST_H