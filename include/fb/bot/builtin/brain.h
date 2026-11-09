#ifndef FB_BOT_BUILTIN_BRAIN_H
#define FB_BOT_BUILTIN_BRAIN_H

#include <fb/bot/brain/brain.h>

struct lua_State;

namespace fb::bot::builtin {

struct brain
{
    static int builtin_state(lua_State* L);
    static int builtin_stat(lua_State* L);
    static int builtin_clock(lua_State* L);
    static int builtin_states(lua_State* L);
    static int builtin_progress(lua_State* L);
    static int builtin_npc(lua_State* L);
    static int builtin_ally(lua_State* L);
    static int builtin_persona(lua_State* L);
    static int builtin_active(lua_State* L);
    static int builtin_observe(lua_State* L);
    static int builtin_route(lua_State* L);
    static int builtin_reach(lua_State* L);
    static int builtin_warp_at(lua_State* L);
    static int builtin_walk(lua_State* L);
    static int builtin_remember(lua_State* L);
    static int builtin_npcs(lua_State* L);
    static int builtin_talk(lua_State* L);
    static int builtin_death(lua_State* L);
    static int builtin_bury(lua_State* L);
    static int builtin_inventory(lua_State* L);
    static int builtin_loot(lua_State* L);
    static int builtin_watch(lua_State* L);
    static int builtin_pick(lua_State* L);
    static int builtin_items(lua_State* L);
    static int builtin_concern(lua_State* L);
    static int builtin_failure(lua_State* L);
    static int builtin_activities(lua_State* L);
    static int builtin_expect(lua_State* L);
    static int builtin_trace(lua_State* L);
    static int builtin_grid(lua_State* L);
    static int builtin_steps(lua_State* L);
    static int builtin_worn(lua_State* L);
    static int builtin_item(lua_State* L);
    static int builtin_reserve(lua_State* L);
    static int builtin_release(lua_State* L);
    static int builtin_reservations(lua_State* L);
    static int builtin_occupy(lua_State* L);
    static int builtin_demand(lua_State* L);
    static int builtin_post(lua_State* L);
    static int builtin_roster(lua_State* L);
    static int builtin_recall(lua_State* L);
    static int builtin_mobs(lua_State* L);
    static int builtin_spawns(lua_State* L);
    static int builtin_native(lua_State* L);
    static int builtin_habitats(lua_State* L);
    static int builtin_engage(lua_State* L);
    static int builtin_cast(lua_State* L);
    static int builtin_aside(lua_State* L);
    static int builtin_tree(lua_State* L);
    static int builtin_leaps(lua_State* L);

private:
    static void                        tree_push(lua_State* L, int node);
    static void                        tree_pop(lua_State* L);
    static int                         tree_walk(lua_State* L);
    static int                         tree_resume(lua_State* L, int status, lua_KContext phase);
    static fb::bot::brain::destination read_destination(lua_State* L, int index);
    static fb::bot::brain::detour      read_detour(lua_State* L, int index);
    static void                        push_mob(lua_State* L, const fb::model::mob& model);
};

} // namespace fb::bot::builtin

#endif
