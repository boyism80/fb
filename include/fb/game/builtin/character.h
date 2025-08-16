#ifndef FB_GAME_BUILTIN_CHARACTER_H
#define FB_GAME_BUILTIN_CHARACTER_H

#include <fb/game/character.h>

namespace fb::game::builtin {

class character
{
public:
    /**
     * @brief      Lua binding for getting/setting character appearance.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_look(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character color.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_color(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character sex.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_sex(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character money.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_money(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character experience.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_exp(lua_State* L);

    /**
     * @brief      Lua binding for getting character item by slot.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_item(lua_State* L);

    /**
     * @brief      Lua binding for getting all character items.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_items(lua_State* L);

    /**
     * @brief      Lua binding for getting character equipment items.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_equipments(lua_State* L);

    /**
     * @brief      Lua binding for dropping character item.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_item_drop(lua_State* L);

    /**
     * @brief      Lua binding for creating item in character inventory.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mkitem(lua_State* L);

    /**
     * @brief      Lua binding for removing item from character inventory.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_rmitem(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character state.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_state(lua_State* L);

    /**
     * @brief      Lua binding for setting character disguise.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_disguise(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character class.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_class(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character promotion level.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_promotion(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character level.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_level(lua_State* L);

    /**
     * @brief      Lua binding for asserting character state.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_assert(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting character role.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_role(lua_State* L);

    /**
     * @brief      Lua binding for getting/setting deposited money.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_deposited_money(lua_State* L);

    /**
     * @brief      Lua binding for getting stored item from bank.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_stored_item(lua_State* L);

    /**
     * @brief      Lua binding for storing item in bank.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_store_item(lua_State* L);

    /**
     * @brief      Lua builtin function to retrieve an item from storage or inventory.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_retrieve_item(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's current group.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_group(lua_State* L);

    /**
     * @brief      Lua builtin function to create a new group with the character as leader.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_create_group(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's current clan.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_clan(lua_State* L);

    /**
     * @brief      Lua builtin function to create a new clan with the character as leader.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_create_clan(lua_State* L);

    /**
     * @brief      Lua builtin function to destroy the character's clan.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_destroy_clan(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's achievements list.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_achievements(lua_State* L);

    /**
     * @brief      Lua builtin function to get a specific achievement by ID.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_achievement(lua_State* L);

    /**
     * @brief      Lua builtin function to add a new achievement to the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_push_achievement(lua_State* L);

    /**
     * @brief      Lua builtin function to remove an achievement from the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_erase_achievement(lua_State* L);

    /**
     * @brief      Lua builtin function to send a whisper message to another character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_whisper(lua_State* L);

    /**
     * @brief      Lua builtin function to send mail to another character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_send_mail(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's nation affiliation.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_nation(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's equipped weapon.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_weapon(lua_State* L);

    /**
     * @brief      Lua builtin function to get or set the character's title.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_title(lua_State* L);

    /**
     * @brief      Lua builtin function to add experience or stats to the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_gain(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's weapon damage.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_weapon_damage(lua_State* L);

    /**
     * @brief      Lua builtin function to detect hidden or invisible objects/characters.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_detect(lua_State* L);

    /**
     * @brief      Lua builtin function to spawn a mob at a specific location.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_spawn_mob(lua_State* L);

    /**
     * @brief      Lua builtin function to get a list of mobs spawned by the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_spawned_mobs(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's base HP value.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_base_hp(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's base MP value.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_base_mp(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's base strength value.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_base_str(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's base dexterity value.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_base_dex(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's base intelligence value.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_base_int(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's base damage value.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_base_dam(lua_State* L);

    /**
     * @brief      Lua builtin function to get the character's base hit rate value.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_base_hit(lua_State* L);

    /**
     * @brief      Lua builtin function to get or set the character's armor color.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_armor_color(lua_State* L);

    /**
     * @brief      Lua builtin function to add a spell to the character's spellbook.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_mkspell(lua_State* L);

    /**
     * @brief      Lua builtin function to remove a spell from the character's spellbook.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_rmspell(lua_State* L);

    /**
     * @brief      Lua builtin function to get the world/map the character is in.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_world(lua_State* L);

    /**
     * @brief      Lua builtin function to execute a script or get script information.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_script(lua_State* L);

    /**
     * @brief      Lua builtin function to display an advertisement or announcement.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_ad(lua_State* L);

    /**
     * @brief      Lua builtin function to open a web page or URL for the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_web(lua_State* L);

    /**
     * @brief      Lua builtin function to add a delay or wait time.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_delay(lua_State* L);

    /**
     * @brief      Lua builtin function to get or set the character's birthday.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_birthday(lua_State* L);

    /**
     * @brief      Lua builtin function to check if the character is active or online.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_active(lua_State* L);

    /**
     * @brief      Lua builtin function to enable super hide mode for the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_super_hide(lua_State* L);

    /**
     * @brief      Lua builtin function to transform the character into a creature.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_creature(lua_State* L);

    /**
     * @brief      Lua builtin function to teleport the character to a location.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_teleport(lua_State* L);

    /**
     * @brief      Lua builtin function to display a dialog to the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_dialog(lua_State* L);

    /**
     * @brief      Lua builtin function to display a list interface to the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_list(lua_State* L);

    /**
     * @brief      Lua builtin function to display an input dialog to the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_input(lua_State* L);

    /**
     * @brief      Lua builtin function to display a menu interface to the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_menu(lua_State* L);

    /**
     * @brief      Lua builtin function to access or manipulate inventory slots.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_slot(lua_State* L);

    /**
     * @brief      Lua builtin function to force a rezen spawn.
     *
     *             This function forces a rezen spawn regardless of normal spawn
     *             conditions. It bypasses the normal spawn restrictions and
     *             immediately spawns mobs at the specified rezen point.
     *
     *             This function is useful for special events, GM commands, or
     *             scripted scenarios that need to override normal spawn logic.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_rezen_force(lua_State* L);

    /**
     * @brief      Lua builtin function to get a quest by ID.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_quest(lua_State* L);

    /**
     * @brief      Lua builtin function to start a quest for the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_start_quest(lua_State* L);

    /**
     * @brief      Lua builtin function to remove a quest from the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_remove_quest(lua_State* L);

    /**
     * @brief      Lua builtin function to check if a quest can be started for the character.
     *
     * @param      L     The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to the Lua stack.
     */
    static int builtin_can_start_quest(lua_State* L);
};

} // namespace fb::game::builtin

#endif // !FB_GAME_BUILTIN_CHARACTER_H