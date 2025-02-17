#include <fb/model/model.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::spell, "fb_model_spell")
{"type",                fb::model::spell::builtin_type},
{"name",                fb::model::spell::builtin_name},
{"message",             fb::model::spell::builtin_message},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::map, "fb_model_map")
{"id",                  fb::model::map::builtin_id},
{"name",                fb::model::map::builtin_name},
{"root",                fb::model::map::builtin_root},
{"cardinal",            fb::model::map::builtin_cardinal},
{"resurrection",        fb::model::map::builtin_resurrection},
{"option",              fb::model::map::builtin_option},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::object, "fb_model_object")
{"name",                fb::model::object::builtin_name},
{"look",                fb::model::object::builtin_look},
{"color",               fb::model::object::builtin_color},
{"dialog",              fb::model::object::builtin_dialog},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::mob, "fb_model_mob")
{"speed",               fb::model::mob::builtin_speed},
{"size",                fb::model::mob::builtin_size},
{"damage",              fb::model::mob::builtin_damage},
{"drop",                fb::model::mob::builtin_drop},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::npc, "fb_model_npc")
{"input",               fb::model::npc::builtin_input},
{"menu",                fb::model::npc::builtin_menu},
{"list",                fb::model::npc::builtin_list},
{"item",                fb::model::npc::builtin_item},
{"slot",                fb::model::npc::builtin_slot},
{"sell",                fb::model::npc::builtin_sell},
{"sell_price",          fb::model::npc::builtin_sell_price},
{"buy",                 fb::model::npc::builtin_buy},
{"buy_price",           fb::model::npc::builtin_buy_price},
{"repair",              fb::model::npc::builtin_repair},
{"repair_all",          fb::model::npc::builtin_repair_all},
{"hold_money",          fb::model::npc::builtin_hold_money},
{"hold_item",           fb::model::npc::builtin_hold_item},
{"return_money",        fb::model::npc::builtin_return_money},
{"return_item",         fb::model::npc::builtin_return_item},
{"rename_weapon",       fb::model::npc::builtin_rename_weapon},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::life, "fb_model_life")
{"hp",                  fb::model::life::builtin_hp},
{"mp",                  fb::model::life::builtin_mp},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::item, "fb_model_item")
{"make",                fb::model::item::builtin_make},
{"attr",                fb::model::item::builtin_attr},
{"capacity",            fb::model::item::builtin_capacity},
{"durability",          fb::model::item::builtin_durability},
{"price",               fb::model::item::builtin_price},
{"repair_price",        fb::model::item::builtin_repair_price},
{"rename_price",        fb::model::item::builtin_rename_price},
{"deposit_price",       fb::model::item::builtin_deposit_price},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::weapon, "fb_model_weapon")
{"damage_small",        fb::model::weapon::builtin_damage_small},
{"damage_large",        fb::model::weapon::builtin_damage_large},
{"sound",               fb::model::weapon::builtin_sound},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::trace, "fb_model_trace")
{"id",                  fb::model::trace::builtin_id},
{"look",                fb::model::trace::builtin_look},
{"color",               fb::model::trace::builtin_color},
{"text",                fb::model::trace::builtin_text},
END_LUA_EXTENSION; // clang-format on