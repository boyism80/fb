#include "context.h"

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::context, "")
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::lua::luable, "fb.luable")
{"__gc",                fb::lua::luable::builtin_gc},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::spell, "fb.model.spell")
{"type",                fb::model::spell::builtin_type},
{"name",                fb::model::spell::builtin_name},
{"message",             fb::model::spell::builtin_message},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::map, "fb.game.map")
{"width",               fb::game::map::builtin_width},
{"height",              fb::game::map::builtin_height},
{"area",                fb::game::map::builtin_area},
{"objects",             fb::game::map::builtin_objects},
{"movable",             fb::game::map::builtin_movable},
{"door",                fb::game::map::builtin_door},
{"doors",               fb::game::map::builtin_doors},
{"contains",            fb::game::map::builtin_contains},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::map, "fb.model.map")
{"name",                fb::model::map::builtin_name},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::object, "fb.model.object")
{"name",                fb::model::object::builtin_name},
{"look",                fb::model::object::builtin_look},
{"color",               fb::model::object::builtin_color},
{"dialog",              fb::model::object::builtin_dialog},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::mob, "fb.model.mob")
{"speed",               fb::model::mob::builtin_speed},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::mob, "fb.game.mob")
{"__eq",                fb::game::object::builtin_eq},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::npc, "fb.model.npc")
{"input",               fb::model::npc::builtin_input},
{"menu",                fb::model::npc::builtin_menu},
{"item",                fb::model::npc::builtin_item},
{"slot",                fb::model::npc::builtin_slot},
{"sell",                fb::model::npc::builtin_sell},
{"buy",                 fb::model::npc::builtin_buy},
{"repair",              fb::model::npc::builtin_repair},
{"repair_all",          fb::model::npc::builtin_repair_all},
{"hold_money",          fb::model::npc::builtin_hold_money},
{"hold_item",           fb::model::npc::builtin_hold_item},
{"return_money",        fb::model::npc::builtin_return_money},
{"return_item",         fb::model::npc::builtin_return_item},
{"rename_weapon",       fb::model::npc::builtin_rename_weapon},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::npc, "fb.game.npc")
END_LUA_EXTENSION


IMPLEMENT_LUA_EXTENSION(fb::game::object, "fb.game.object")
{"model",               fb::game::object::builtin_model},
{"__eq",                fb::game::object::builtin_eq},
{"__tostring",          fb::game::object::builtin_tostring},
{"id",                  fb::game::object::builtin_id},
{"name",                fb::game::object::builtin_name},
{"sound",               fb::game::object::builtin_sound},
{"position",            fb::game::object::builtin_position},
{"direction",           fb::game::object::builtin_direction},
{"chat",                fb::game::object::builtin_chat},
{"message",             fb::game::object::builtin_message},
{"buff",                fb::game::object::builtin_buff},
{"isbuff",              fb::game::object::builtin_isbuff},
{"unbuff",              fb::game::object::builtin_unbuff},
{"effect",              fb::game::object::builtin_effect},
{"map",                 fb::game::object::builtin_map},
{"mkitem",              fb::game::object::builtin_mkitem},
{"showings",            fb::game::object::builtin_showings},
{"showns",              fb::game::object::builtin_showns},
{"front",               fb::game::object::builtin_front},
{"is",                  fb::game::object::builtin_is},
END_LUA_EXTENSION


IMPLEMENT_LUA_EXTENSION(fb::model::life, "fb.model.life")
{"hp",                  fb::model::life::builtin_hp},
{"mp",                  fb::model::life::builtin_mp},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::life, "fb.game.life")
{"__eq",                fb::game::object::builtin_eq},
{"hp",                  fb::game::life::builtin_hp},
{"mp",                  fb::game::life::builtin_mp},
{"hp_inc",              fb::game::life::builtin_hp_inc},
{"mp_inc",              fb::game::life::builtin_mp_inc},
{"hp_dec",              fb::game::life::builtin_hp_dec},
{"mp_dec",              fb::game::life::builtin_mp_dec},
{"base_hp",             fb::game::life::builtin_base_hp},
{"base_mp",             fb::game::life::builtin_base_mp},
{"action",              fb::game::life::builtin_action},
{"spell",               fb::game::life::builtin_spell},
{"damage",              fb::game::life::builtin_damage},
{"cast",                fb::game::life::builtin_cast},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::model::item, "fb.model.item")
{"make",                fb::model::item::builtin_make},
{"attr",                fb::model::item::builtin_attr},
{"capacity",            fb::model::item::builtin_capacity},
{"durability",          fb::model::item::builtin_durability},
{"price",               fb::model::item::builtin_price},
{"repair_price",        fb::model::item::builtin_repair_price},
{"rename_price",        fb::model::item::builtin_rename_price},
{"deposit_price",       fb::model::item::builtin_deposit_price},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::item, "fb.game.item")
{"model",               fb::game::item::builtin_model},
{"count",               fb::game::item::builtin_count},
{"durability",          fb::game::item::builtin_durability},
{"rename",              fb::game::item::builtin_rename},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::character, "fb.game.session")
{"__eq",                fb::game::object::builtin_eq},
{"look",                fb::game::character::builtin_look},
{"color",               fb::game::character::builtin_color},
{"money",               fb::game::character::builtin_money},
{"exp",                 fb::game::character::builtin_exp},
{"base_hp",             fb::game::character::builtin_base_hp},
{"base_mp",             fb::game::character::builtin_base_mp},
{"str",                 fb::game::character::builtin_strength},
{"dex",                 fb::game::character::builtin_dexterity},
{"int",                 fb::game::character::builtin_intelligence},
{"item",                fb::game::character::builtin_item},
{"items",               fb::game::character::builtin_items},
{"dropitem",            fb::game::character::builtin_item_drop},
{"mkitem",              fb::game::character::builtin_mkitem},
{"rmitem",              fb::game::character::builtin_rmitem},
{"state",               fb::game::character::builtin_state},
{"disguise",            fb::game::character::builtin_disguise},
{"class",               fb::game::character::builtin_class},
{"level",               fb::game::character::builtin_level},
{"assert",              fb::game::character::builtin_assert},
{"deposited_money",     fb::game::character::builtin_deposited_money},
{"deposited_item",      fb::game::character::builtin_deposited_item},
{"deposit_item",        fb::game::character::builtin_deposit_item},
{"withdraw_item",       fb::game::character::builtin_withdraw_item},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::door, "fb.game.door")
{"toggle",              fb::game::door::builtin_toggle},
{"lock",                fb::game::door::builtin_lock},
{"locked",              fb::game::door::builtin_locked},
{"opened",              fb::game::door::builtin_opened},
END_LUA_EXTENSION