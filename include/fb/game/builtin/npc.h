#ifndef FB_GAME_BUILTIN_NPC_H
#define FB_GAME_BUILTIN_NPC_H

#include <fb/game/npc.h>

namespace fb::game::builtin {

struct npc
{
    static int builtin_appearance(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_NPC_H
