#ifndef FB_GAME_BUILTIN_CHARACTER_H
#define FB_GAME_BUILTIN_CHARACTER_H

#include <fb/game/character.h>

namespace fb::game::builtin {

struct character
{
    static int builtin_uid(lua_State* L);
    static int builtin_look(lua_State* L);
    static int builtin_color(lua_State* L);
    static int builtin_gender(lua_State* L);
    static int builtin_money(lua_State* L);
    static int builtin_exp(lua_State* L);
    static int builtin_item(lua_State* L);
    static int builtin_items(lua_State* L);
    static int builtin_has_items(lua_State* L);
    static int builtin_equipments(lua_State* L);
    static int builtin_equipment_off(lua_State* L);
    static int builtin_item_drop(lua_State* L);
    static int builtin_mkitem(lua_State* L);
    static int builtin_rmitem(lua_State* L);
    static int builtin_preview_item_looks(lua_State* L);
    static int builtin_exchange(lua_State* L);
    static int builtin_state(lua_State* L);
    static int builtin_mimic(lua_State* L);
    static int builtin_class(lua_State* L);
    static int builtin_promotion(lua_State* L);
    static int builtin_level(lua_State* L);
    static int builtin_assert(lua_State* L);
    static int builtin_role(lua_State* L);
    static int builtin_deposited_money(lua_State* L);
    static int builtin_stored_item(lua_State* L);
    static int builtin_store_item(lua_State* L);
    static int builtin_retrieve_item(lua_State* L);
    static int builtin_group(lua_State* L);
    static int builtin_create_group(lua_State* L);
    static int builtin_clan(lua_State* L);
    static int builtin_create_clan(lua_State* L);
    static int builtin_destroy_clan(lua_State* L);
    static int builtin_achievements(lua_State* L);
    static int builtin_achievement(lua_State* L);
    static int builtin_push_achievement(lua_State* L);
    static int builtin_erase_achievement(lua_State* L);
    static int builtin_whisper(lua_State* L);
    static int builtin_send_mail(lua_State* L);
    static int builtin_nation(lua_State* L);
    static int builtin_option(lua_State* L);
    static int builtin_weapon(lua_State* L);
    static int builtin_title(lua_State* L);
    static int builtin_gain(lua_State* L);
    static int builtin_weapon_damage(lua_State* L);
    static int builtin_detect(lua_State* L);
    static int builtin_spawn_mob(lua_State* L);
    static int builtin_spawned_mobs(lua_State* L);
    static int builtin_base_hp(lua_State* L);
    static int builtin_base_mp(lua_State* L);
    static int builtin_base_str(lua_State* L);
    static int builtin_base_dex(lua_State* L);
    static int builtin_base_int(lua_State* L);
    static int builtin_base_dam(lua_State* L);
    static int builtin_base_hit(lua_State* L);
    static int builtin_base_speed(lua_State* L);
    static int builtin_armor_color(lua_State* L);
    static int builtin_weapon_color(lua_State* L);
    static int builtin_shield_color(lua_State* L);
    static int builtin_mkspell(lua_State* L);
    static int builtin_rmspell(lua_State* L);
    static int builtin_world(lua_State* L);
    static int builtin_ad(lua_State* L);
    static int builtin_web(lua_State* L);
    static int builtin_timer(lua_State* L);
    static int builtin_delay(lua_State* L);
    static int builtin_birthday(lua_State* L);
    static int builtin_active(lua_State* L);
    static int builtin_super_hide(lua_State* L);
    static int builtin_creature(lua_State* L);
    static int builtin_teleport(lua_State* L);
    static int builtin_dialog(lua_State* L);
    static int builtin_list(lua_State* L);
    static int builtin_input(lua_State* L);
    static int builtin_menu(lua_State* L);
    static int builtin_slot(lua_State* L);
    static int builtin_pursuit(lua_State* L);
    static int builtin_spell(lua_State* L);
    static int builtin_email(lua_State* L);
    static int builtin_dialog_0x30_10(lua_State* L);
    static int builtin_rezen_force(lua_State* L);
    static int builtin_quest(lua_State* L);
    static int builtin_matchmaker(lua_State* L);
    static int builtin_start_quest(lua_State* L);
    static int builtin_remove_quest(lua_State* L);
    static int builtin_reward(lua_State* L);
    static int builtin_send_system_mail(lua_State* L);
    static int builtin_send_storage_box(lua_State* L);
    static int builtin_send_system_storage_box(lua_State* L);
    static int builtin_storage_entries(lua_State* L);
    static int builtin_receive_storage_reward(lua_State* L);
    static int builtin_marketplace_list(lua_State* L);
    static int builtin_marketplace_cancel(lua_State* L);
    static int builtin_marketplace_purchase(lua_State* L);
    static int builtin_marketplace_search(lua_State* L);
    static int builtin_marketplace_get_listings(lua_State* L);
    static int builtin_marketplace_pending_listings(lua_State* L);
    static int builtin_marriage(lua_State* L);
    static int builtin_marry(lua_State* L);
    static int builtin_divorce(lua_State* L);
    static int builtin_unknown_12(lua_State* L);
    static int builtin_move_confirm_noscroll(lua_State* L);
    static int builtin_ui(lua_State* L);
    static int builtin_item_throw_confirm(lua_State* L);
    static int builtin_freeze(lua_State* L);
    static int builtin_friends_sync(lua_State* L);
    static int builtin_c2s_relay(lua_State* L);
    static int builtin_user_info(lua_State* L);
    static int builtin_popup_message(lua_State* L);
    static int builtin_popup_input(lua_State* L);
    static int builtin_holyday_screen(lua_State* L);
};

} // namespace fb::game::builtin

#endif // !FB_GAME_BUILTIN_CHARACTER_H