#include <fb/model/model.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::spell, "fb.model.spell")
{"type",                fb::model::spell::builtin_type},
{"name",                fb::model::spell::builtin_name},
{"message",             fb::model::spell::builtin_message},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::map, "fb.model.map")
{"id",                  fb::model::map::builtin_id},
{"name",                fb::model::map::builtin_name},
{"root",                fb::model::map::builtin_root},
{"cardinal",            fb::model::map::builtin_cardinal},
{"resurrection",        fb::model::map::builtin_resurrection},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::object, "fb.model.object")
{"name",                fb::model::object::builtin_name},
{"look",                fb::model::object::builtin_look},
{"color",               fb::model::object::builtin_color},
{"dialog",              fb::model::object::builtin_dialog},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::mob, "fb.model.mob")
{"speed",               fb::model::mob::builtin_speed},
{"size",                fb::model::mob::builtin_size},
{"damage",              fb::model::mob::builtin_damage},
{"drop",                fb::model::mob::builtin_drop},
END_LUA_EXTENSION; // clang-format on

// clang-format off
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
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::life, "fb.model.life")
{"hp",                  fb::model::life::builtin_hp},
{"mp",                  fb::model::life::builtin_mp},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::item, "fb.model.item")
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
IMPLEMENT_LUA_EXTENSION(fb::model::weapon, "fb.model.weapon")
{"damage_small",        fb::model::weapon::builtin_damage_small},
{"damage_large",        fb::model::weapon::builtin_damage_large},
{"sound",        		fb::model::weapon::builtin_sound},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::trace, "fb.model.trace")
{"id",                  fb::model::trace::builtin_id},
{"look",                fb::model::trace::builtin_look},
{"color",               fb::model::trace::builtin_color},
{"text",                fb::model::trace::builtin_text},
END_LUA_EXTENSION; // clang-format on