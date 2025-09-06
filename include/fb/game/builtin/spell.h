#ifndef FB_GAME_BUILTIN_SPELL_H
#define FB_GAME_BUILTIN_SPELL_H

#include <fb/game/spell.h>

namespace fb::game::builtin {

struct spell
{
    static int builtin_model(lua_State* L);
    static int builtin_delay(lua_State* L);
    static int builtin_delay2(lua_State* L);
};

struct buff
{
    static int builtin_model(lua_State* L);
    static int builtin_time(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_SPELL_H
