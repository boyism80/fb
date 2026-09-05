#ifndef FB_BOT_BUILTIN_GAME_BOT_H
#define FB_BOT_BUILTIN_GAME_BOT_H

#include <fb/bot/game_bot.h>

struct lua_State;

namespace fb::bot::builtin {

struct game_bot
{
    static int builtin_oid(lua_State* L);
    static int builtin_name(lua_State* L);
    static int builtin_state(lua_State* L);
    static int builtin_position(lua_State* L);
    static int builtin_money(lua_State* L);
    static int builtin_level(lua_State* L);
    static int builtin_hp(lua_State* L);
    static int builtin_mp(lua_State* L);
    static int builtin_crowd_control(lua_State* L);
    static int builtin_base_hp(lua_State* L);
    static int builtin_base_mp(lua_State* L);
    static int builtin_str(lua_State* L);
    static int builtin_dex(lua_State* L);
    static int builtin_int(lua_State* L);
    static int builtin_exp(lua_State* L);
    static int builtin_gender(lua_State* L);
    static int builtin_map(lua_State* L);
    static int builtin_item_slot(lua_State* L);
    static int builtin_item_count(lua_State* L);
    static int builtin_spell_slot(lua_State* L);
    static int builtin_has_item(lua_State* L);
    static int builtin_has_spell(lua_State* L);
    static int builtin_inventory_size(lua_State* L);
    static int builtin_has_item_by_name(lua_State* L);
    static int builtin_item_base_price(lua_State* L);
    static int builtin_remove_buffs(lua_State* L);
    static int builtin_has_buff(lua_State* L);
    static int builtin_request(lua_State* L);
    static int builtin_request_on(lua_State* L);
    static int builtin_send(lua_State* L);
    static int builtin_chat(lua_State* L);
    static int builtin_move(lua_State* L);
    static int builtin_direction(lua_State* L);
    static int builtin_map_move(lua_State* L);
    static int builtin_transfer(lua_State* L);
    static int builtin_create_item(lua_State* L);
    static int builtin_equip(lua_State* L);
    static int builtin_unequip(lua_State* L);
    static int builtin_clear_inventory(lua_State* L);
    static int builtin_learn_spell(lua_State* L);
    static int builtin_learn_spells(lua_State* L);
    static int builtin_clear_all_spells(lua_State* L);
    static int builtin_setup_bot_stats(lua_State* L);
    static int builtin_set_current_hp_mp(lua_State* L);
    static int builtin_set_max_hp_mp(lua_State* L);
    static int builtin_spawn_monster(lua_State* L);
    static int builtin_spawn_monster_relative(lua_State* L);
    static int builtin_spawn_monsters_bulk(lua_State* L);
    static int builtin_spawn_monsters_relative(lua_State* L);
    static int builtin_move_back(lua_State* L);
    static int builtin_sleep(lua_State* L);
    static int builtin_drop_item(lua_State* L);
    static int builtin_drop_money(lua_State* L);
    static int builtin_change_class(lua_State* L);
    static int builtin_item_condition(lua_State* L);
    static int builtin_reverse_item_condition(lua_State* L);
    static int builtin_invite_group(lua_State* L);
    static int builtin_leave_group(lua_State* L);
    static int builtin_kick_group(lua_State* L);
    static int builtin_fill_inventory(lua_State* L);
    static int builtin_create_npc(lua_State* L);
    static int builtin_clear_all_drop_items(lua_State* L);
    static int builtin_request_dialog(lua_State* L);
    static int builtin_request_dialog_ext(lua_State* L);
    static int builtin_request_trade(lua_State* L);
    static int builtin_bulletin_write(lua_State* L);
    static int builtin_bulletin_get_sections(lua_State* L);
    static int builtin_bulletin_get_articles(lua_State* L);
    static int builtin_bulletin_read_article(lua_State* L);
    static int builtin_bulletin_delete_article(lua_State* L);
    static int builtin_bulletin_send_mail(lua_State* L);
    static int builtin_bulletin_get_mails(lua_State* L);
    static int builtin_bulletin_read_mail(lua_State* L);
    static int builtin_bulletin_delete_mail(lua_State* L);
};

} // namespace fb::bot::builtin

#endif // FB_BOT_BUILTIN_GAME_BOT_H
