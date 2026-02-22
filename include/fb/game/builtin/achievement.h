#ifndef FB_GAME_BUILTIN_ACHIEVEMENT_H
#define FB_GAME_BUILTIN_ACHIEVEMENT_H

#include <fb/game/achievement.h>

namespace fb::game::builtin {

struct achievement
{
    static int builtin_id(lua_State* L);
    static int builtin_text(lua_State* L);
    static int builtin_icon(lua_State* L);
    static int builtin_color(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_ACHIEVEMENT_H
