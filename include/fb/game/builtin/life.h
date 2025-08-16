#ifndef FB_GAME_BUILTIN_LIFE_H
#define FB_GAME_BUILTIN_LIFE_H

#include <fb/game/life.h>

namespace fb::game::builtin {

class life
{
public:
    /**
     * @brief      Lua binding for sending messages to life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_message(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting HP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_hp(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting MP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mp(lua_State* L);

    /**
     * @brief      Lua binding for healing life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_heal(lua_State* L);

    /**
     * @brief      Lua binding for dealing damage to life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_damage(lua_State* L);

    /**
     * @brief      Lua binding for restoring MP to life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mp_up(lua_State* L);

    /**
     * @brief      Lua binding for consuming MP from life entities.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mp_down(lua_State* L);

    /**
     * @brief      Lua binding for performing actions.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_action(lua_State* L);

    /**
     * @brief      Lua binding for accessing individual spells.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_spell(lua_State* L);

    /**
     * @brief      Lua binding for accessing the spell collection.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_spells(lua_State* L);

    /**
     * @brief      Lua binding for casting spells.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_cast(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting crowd control effects.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_cc(lua_State* L);

    /**
     * @brief      Lua binding for adding crowd control effects.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_add_cc(lua_State* L);

    /**
     * @brief      Lua binding for removing crowd control effects.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_remove_cc(lua_State* L);

    /**
     * @brief      Lua binding for performing attacks.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_attack(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting damage rate multipliers.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_damage_rate(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting skill damage rate multipliers.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_skill_damage_rate(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting damage reduction rates.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_damage_derate(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting paralysis status.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_paralysis(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting invincibility status.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_invincible(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting cover status.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_cover(lua_State* L);

    /**
     * @brief      Lua binding for getting base HP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_hp(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting HP buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_hp(lua_State* L);

    /**
     * @brief      Lua binding for getting maximum HP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_maxhp(lua_State* L);

    /**
     * @brief      Lua binding for getting base MP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_mp(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting MP buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_mp(lua_State* L);

    /**
     * @brief      Lua binding for getting maximum MP values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_maxmp(lua_State* L);

    /**
     * @brief      Lua binding for getting base strength values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_str(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting strength buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_str(lua_State* L);

    /**
     * @brief      Lua binding for getting total strength values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_str(lua_State* L);

    /**
     * @brief      Lua binding for getting base dexterity values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_dex(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting dexterity buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_dex(lua_State* L);

    /**
     * @brief      Lua binding for getting total dexterity values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_dex(lua_State* L);

    /**
     * @brief      Lua binding for getting base intelligence values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_int(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting intelligence buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_int(lua_State* L);

    /**
     * @brief      Lua binding for getting total intelligence values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_intelligence(lua_State* L);

    /**
     * @brief      Lua binding for getting base physical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_phydef(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting physical defense buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_phydef(lua_State* L);

    /**
     * @brief      Lua binding for getting total physical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_phydef(lua_State* L);

    /**
     * @brief      Lua binding for getting base magical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_magdef(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting magical defense buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_magdef(lua_State* L);

    /**
     * @brief      Lua binding for getting total magical defense values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_magdef(lua_State* L);

    /**
     * @brief      Lua binding for getting base damage values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_dam(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting damage buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_dam(lua_State* L);

    /**
     * @brief      Lua binding for getting total damage values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_dam(lua_State* L);

    /**
     * @brief      Lua binding for getting base hit accuracy values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_base_hit(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting hit accuracy buff values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff_hit(lua_State* L);

    /**
     * @brief      Lua binding for getting total hit accuracy values.
     *
     * @param      L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_hit(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_LIFE_H