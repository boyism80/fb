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
{"revive",              fb::model::map::builtin_revive},
{"option",              fb::model::map::builtin_option},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::object, "fb.model.object")
{"name",                fb::model::object::builtin_name},
{"look",                fb::model::object::builtin_look},
{"color",               fb::model::object::builtin_color},
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
{"sell",                fb::model::npc::builtin_sell},
{"sell_price",          fb::model::npc::builtin_sell_price},
{"buy",                 fb::model::npc::builtin_buy},
{"buy_price",           fb::model::npc::builtin_buy_price},
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
{"storage_fee",         fb::model::item::builtin_storage_fee},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::weapon, "fb.model.weapon")
{"damage_small",        fb::model::weapon::builtin_damage_small},
{"damage_large",        fb::model::weapon::builtin_damage_large},
{"sound",               fb::model::weapon::builtin_sound},
{"type",                fb::model::weapon::builtin_type},
END_LUA_EXTENSION; // clang-format on

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::trace, "fb.model.trace")
{"id",                  fb::model::trace::builtin_id},
{"look",                fb::model::trace::builtin_look},
{"color",               fb::model::trace::builtin_color},
{"text",                fb::model::trace::builtin_text},
END_LUA_EXTENSION; // clang-format on