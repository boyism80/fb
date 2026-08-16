#ifndef FB_GAME_BUILTIN_LIFE_H
#define FB_GAME_BUILTIN_LIFE_H

#include <fb/game/life.h>

namespace fb::game::builtin {

struct life
{
    static int builtin_message(lua_State* L);
    static int builtin_hp(lua_State* L);
    static int builtin_mp(lua_State* L);
    static int builtin_heal(lua_State* L);
    static int builtin_damage_to(lua_State* L);
    static int builtin_mp_up(lua_State* L);
    static int builtin_mp_down(lua_State* L);
    static int builtin_action(lua_State* L);
    static int builtin_spell(lua_State* L);
    static int builtin_spells(lua_State* L);
    static int builtin_cast(lua_State* L);
    static int builtin_cc(lua_State* L);
    static int builtin_add_cc(lua_State* L);
    static int builtin_remove_cc(lua_State* L);
    static int builtin_attack(lua_State* L);
    static int builtin_damage_rate(lua_State* L);
    static int builtin_skill_damage_rate(lua_State* L);
    static int builtin_damage_derate(lua_State* L);
    static int builtin_paralysis(lua_State* L);
    static int builtin_invincible(lua_State* L);
    static int builtin_cover(lua_State* L);
    static int builtin_delirious(lua_State* L);
    static int builtin_base_hp(lua_State* L);
    static int builtin_buff_hp(lua_State* L);
    static int builtin_buff_hp_percent(lua_State* L);
    static int builtin_maxhp(lua_State* L);
    static int builtin_base_mp(lua_State* L);
    static int builtin_buff_mp(lua_State* L);
    static int builtin_buff_mp_percent(lua_State* L);
    static int builtin_maxmp(lua_State* L);
    static int builtin_base_str(lua_State* L);
    static int builtin_buff_str(lua_State* L);
    static int builtin_str(lua_State* L);
    static int builtin_base_dex(lua_State* L);
    static int builtin_buff_dex(lua_State* L);
    static int builtin_dex(lua_State* L);
    static int builtin_base_int(lua_State* L);
    static int builtin_buff_int(lua_State* L);
    static int builtin_intelligence(lua_State* L);
    static int builtin_base_phydef(lua_State* L);
    static int builtin_buff_phydef(lua_State* L);
    static int builtin_phydef(lua_State* L);
    static int builtin_base_magdef(lua_State* L);
    static int builtin_buff_magdef(lua_State* L);
    static int builtin_magdef(lua_State* L);
    static int builtin_base_resist(lua_State* L);
    static int builtin_buff_resist(lua_State* L);
    static int builtin_resist(lua_State* L);
    static int builtin_base_dam(lua_State* L);
    static int builtin_buff_dam(lua_State* L);
    static int builtin_dam(lua_State* L);
    static int builtin_base_hit(lua_State* L);
    static int builtin_buff_hit(lua_State* L);
    static int builtin_hit(lua_State* L);
    static int builtin_base_speed(lua_State* L);
    static int builtin_buff_speed(lua_State* L);
    static int builtin_speed(lua_State* L);
    static int builtin_normal_attack_damage(lua_State* L);
    static int builtin_update(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_LIFE_H