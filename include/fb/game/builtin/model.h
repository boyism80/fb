#ifndef FB_GAME_BUILTIN_MODEL_H
#define FB_GAME_BUILTIN_MODEL_H

#include <fb/lua.h>

namespace fb::game::builtin::model {

struct object
{
    static int builtin_name(lua_State* L);
    static int builtin_look(lua_State* L);
    static int builtin_color(lua_State* L);
};

struct life
{
    static int builtin_hp(lua_State* L);
    static int builtin_mp(lua_State* L);
};

struct item
{
    static int builtin_make(lua_State* L);
    static int builtin_attr(lua_State* L);
    static int builtin_capacity(lua_State* L);
    static int builtin_durability(lua_State* L);
    static int builtin_price(lua_State* L);
    static int builtin_repair_price(lua_State* L);
    static int builtin_rename_price(lua_State* L);
    static int builtin_storage_fee(lua_State* L);
};

struct equipment
{
    static int builtin_dress(lua_State* L);
};

struct weapon
{
    static int builtin_damage_small(lua_State* L);
    static int builtin_damage_large(lua_State* L);
    static int builtin_sound(lua_State* L);
    static int builtin_type(lua_State* L);
};

struct npc
{
    static int builtin_sell(lua_State* L);
    static int builtin_sell_price(lua_State* L);
    static int builtin_buy(lua_State* L);
    static int builtin_buy_price(lua_State* L);
    static int builtin_interaction(lua_State* L);
};

struct mob
{
    static int builtin_speed(lua_State* L);
    static int builtin_size(lua_State* L);
    static int builtin_damage(lua_State* L);
    static int builtin_drop(lua_State* L);
};

struct map
{
    static int builtin_id(lua_State* L);
    static int builtin_name(lua_State* L);
    static int builtin_root(lua_State* L);
    static int builtin_cardinal(lua_State* L);
    static int builtin_revive(lua_State* L);
    static int builtin_option(lua_State* L);
};

struct spell
{
    static int builtin_type(lua_State* L);
    static int builtin_name(lua_State* L);
    static int builtin_message(lua_State* L);
};

struct achievement
{
    static int builtin_id(lua_State* L);
    static int builtin_look(lua_State* L);
    static int builtin_color(lua_State* L);
    static int builtin_text(lua_State* L);
};

struct quest
{
    static int builtin_step(lua_State* L);
    static int builtin_progress(lua_State* L);
    static int builtin_reward(lua_State* L);
};

} // namespace fb::game::builtin::model

#endif // !FB_GAME_BUILTIN_MODEL_H