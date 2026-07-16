#include <fb/bot/builtin/game_bot.h>
#include <fb/bot/game_controller.h>
#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/bot/integration/protocol_registry.h>
#include <fb/logger.h>
#include <fb/model/model.h>

#include <fb/lua.h>

#include <cstring>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::chrono_literals;
using namespace fb::bot::integration;
using namespace fb::model::enum_value;
using namespace fb::bot;

namespace {

#ifdef _DEBUG
constexpr auto INTEGRATION_DEFAULT_TIMEOUT = 30s;
#else
constexpr auto INTEGRATION_DEFAULT_TIMEOUT = 10s;
#endif

void lua_arg_error(lua_State* L, int index, const char* expected)
{
    luaL_error(L, "argument %d must be %s", index, expected);
}

template <typename T>
T require_integer(lua_State* L, fb::lua::context* lua, int index)
{
    if (lua->argc() < index || lua->is_number(index) == false)
        lua_arg_error(L, index, "number");

    return static_cast<T>(lua->tonumber(index));
}

std::string require_string(lua_State* L, fb::lua::context* lua, int index)
{
    if (lua->argc() < index || lua->is_string(index) == false)
        lua_arg_error(L, index, "string");

    return lua->tostring(index);
}

const char* unified_response_opcode_error(uint8_t opcode)
{
    switch (opcode)
    {
    case 0x42:
        return "use bot:request_trade() for trade responses (opcode 0x42)";
    case 0x2F:
        return "use bot:request_dialog() for dialog responses (opcode 0x2F)";
    case 0x30:
        return "use bot:request_dialog_ext() for dialog responses (opcode 0x30)";
    case 0x31:
        return "use bot bulletin methods for bulletin responses (opcode 0x31)";
    default:
        return nullptr;
    }
}

lua_Integer opt_integer(fb::lua::context* lua, int index, lua_Integer default_value)
{
    return lua->tonumber(index, default_value);
}

void push_point(fb::lua::context* lua, const fb::model::point<uint16_t>& point)
{
    lua->new_table();
    lua->pushinteger(1);
    lua->pushinteger(point.x);
    lua->settable(-3);
    lua->pushinteger(2);
    lua->pushinteger(point.y);
    lua->settable(-3);
}

void push_spawned_monster(fb::lua::context* lua, const fb::bot::spawned_monster_info& info)
{
    lua->new_table();
    lua->pushstring("oid");
    lua->pushinteger(info.oid);
    lua->settable(-3);
    lua->pushstring("look");
    lua->pushinteger(info.look);
    lua->settable(-3);
    lua->pushstring("position");
    push_point(lua, info.position);
    lua->settable(-3);
}

std::vector<std::pair<int, int>> parse_relative_positions(fb::lua::context* lua, int index)
{
    std::vector<std::pair<int, int>> positions;
    if (lua->is_table(index) == false)
        return positions;

    for (int i = 1; lua->rawgeti(index, i); lua->pop(1), ++i)
    {
        if (lua->is_table(-1) == false)
            continue;

        lua->rawgeti(-1, 1);
        auto x = static_cast<int>(lua->tonumber(-1));
        lua->pop(1);

        lua->rawgeti(-1, 2);
        auto y = static_cast<int>(lua->tonumber(-1));
        lua->pop(1);

        positions.emplace_back(x, y);
    }

    return positions;
}

std::vector<std::string> parse_string_list(fb::lua::context* lua, int index)
{
    std::vector<std::string> names;
    if (lua == nullptr || lua->is_table(index) == false)
        return names;

    for (int i = 1; lua->rawgeti(index, i); lua->pop(1), ++i)
    {
        if (lua->is_string(-1) == false)
            continue;

        names.emplace_back(lua->tostring(-1));
    }

    return names;
}

fb::model::point<uint16_t> parse_point(fb::lua::context* lua, int index)
{
    lua->rawgeti(index, 1);
    auto x = static_cast<uint16_t>(lua->tonumber(-1));
    lua->pop(1);

    lua->rawgeti(index, 2);
    auto y = static_cast<uint16_t>(lua->tonumber(-1));
    lua->pop(1);

    return {x, y};
}

const fb::model::item* find_item_by_name(std::string_view name)
{
    auto name_str = std::string(name);
    for (auto& [_, item] : fb::model::table::item)
    {
        if (item.name == name_str)
            return &item;
    }
    return nullptr;
}

} // namespace

// clang-format off
IMPLEMENT_LUA_EXTENSION(game_bot, "fb.bot")
    {"oid",                      builtin::game_bot::builtin_oid},
    {"name",                     builtin::game_bot::builtin_name},
    {"state",                    builtin::game_bot::builtin_state},
    {"position",                 builtin::game_bot::builtin_position},
    {"money",                    builtin::game_bot::builtin_money},
    {"level",                    builtin::game_bot::builtin_level},
    {"hp",                       builtin::game_bot::builtin_hp},
    {"mp",                       builtin::game_bot::builtin_mp},
    {"crowd_control",            builtin::game_bot::builtin_crowd_control},
    {"base_hp",                  builtin::game_bot::builtin_base_hp},
    {"base_mp",                  builtin::game_bot::builtin_base_mp},
    {"str",                      builtin::game_bot::builtin_str},
    {"dex",                      builtin::game_bot::builtin_dex},
    {"int",                      builtin::game_bot::builtin_int},
    {"exp",                      builtin::game_bot::builtin_exp},
    {"gender",                   builtin::game_bot::builtin_gender},
    {"map",                      builtin::game_bot::builtin_map},
    {"item_slot",                builtin::game_bot::builtin_item_slot},
    {"item_count",               builtin::game_bot::builtin_item_count},
    {"spell_slot",               builtin::game_bot::builtin_spell_slot},
    {"has_item",                 builtin::game_bot::builtin_has_item},
    {"has_spell",                builtin::game_bot::builtin_has_spell},
    {"inventory_size",           builtin::game_bot::builtin_inventory_size},
    {"has_item_by_name",         builtin::game_bot::builtin_has_item_by_name},
    {"item_base_price",          builtin::game_bot::builtin_item_base_price},
    {"remove_buffs",             builtin::game_bot::builtin_remove_buffs},
    {"has_buff",                 builtin::game_bot::builtin_has_buff},
    {"request",                  builtin::game_bot::builtin_request},
    {"request_on",               builtin::game_bot::builtin_request_on},
    {"send",                     builtin::game_bot::builtin_send},
    {"chat",                     builtin::game_bot::builtin_chat},
    {"move",                     builtin::game_bot::builtin_move},
    {"direction",                builtin::game_bot::builtin_direction},
    {"map_move",                 builtin::game_bot::builtin_map_move},
    {"transfer",                 builtin::game_bot::builtin_transfer},
    {"create_item",              builtin::game_bot::builtin_create_item},
    {"equip",                    builtin::game_bot::builtin_equip},
    {"unequip",                  builtin::game_bot::builtin_unequip},
    {"clear_inventory",          builtin::game_bot::builtin_clear_inventory},
    {"learn_spell",              builtin::game_bot::builtin_learn_spell},
    {"learn_spells",             builtin::game_bot::builtin_learn_spells},
    {"clear_all_spells",         builtin::game_bot::builtin_clear_all_spells},
    {"setup_bot_stats",          builtin::game_bot::builtin_setup_bot_stats},
    {"set_current_hp_mp",        builtin::game_bot::builtin_set_current_hp_mp},
    {"set_max_hp_mp",            builtin::game_bot::builtin_set_max_hp_mp},
    {"spawn_monster",            builtin::game_bot::builtin_spawn_monster},
    {"spawn_monster_relative",   builtin::game_bot::builtin_spawn_monster_relative},
    {"spawn_monsters_bulk",      builtin::game_bot::builtin_spawn_monsters_bulk},
    {"spawn_monsters_relative",  builtin::game_bot::builtin_spawn_monsters_relative},
    {"move_back",                builtin::game_bot::builtin_move_back},
    {"sleep",                    builtin::game_bot::builtin_sleep},
    {"drop_item",                builtin::game_bot::builtin_drop_item},
    {"drop_money",               builtin::game_bot::builtin_drop_money},
    {"change_class",             builtin::game_bot::builtin_change_class},
    {"apply_item_condition",     builtin::game_bot::builtin_apply_item_condition},
    {"reverse_item_condition",   builtin::game_bot::builtin_reverse_item_condition},
    {"invite_group",             builtin::game_bot::builtin_invite_group},
    {"leave_group",              builtin::game_bot::builtin_leave_group},
    {"kick_group",               builtin::game_bot::builtin_kick_group},
    {"fill_inventory",           builtin::game_bot::builtin_fill_inventory},
    {"create_npc",               builtin::game_bot::builtin_create_npc},
    {"clear_all_drop_items",     builtin::game_bot::builtin_clear_all_drop_items},
    {"request_dialog",           builtin::game_bot::builtin_request_dialog},
    {"request_dialog_ext",       builtin::game_bot::builtin_request_dialog_ext},
    {"request_trade",            builtin::game_bot::builtin_request_trade},
    {"bulletin_write",           builtin::game_bot::builtin_bulletin_write},
    {"bulletin_get_sections",    builtin::game_bot::builtin_bulletin_get_sections},
    {"bulletin_get_articles",    builtin::game_bot::builtin_bulletin_get_articles},
    {"bulletin_read_article",    builtin::game_bot::builtin_bulletin_read_article},
    {"bulletin_delete_article",  builtin::game_bot::builtin_bulletin_delete_article},
    {"bulletin_send_mail",       builtin::game_bot::builtin_bulletin_send_mail},
    {"bulletin_get_mails",       builtin::game_bot::builtin_bulletin_get_mails},
    {"bulletin_read_mail",       builtin::game_bot::builtin_bulletin_read_mail},
    {"bulletin_delete_mail",     builtin::game_bot::builtin_bulletin_delete_mail},
END_LUA_EXTENSION;
// clang-format on
// ─── Synchronous getters ──────────────────────────────────────────────────────

int builtin::game_bot::builtin_oid(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(bot->oid()));
    return 1;
}

int builtin::game_bot::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    lua->pushstring(bot->name());
    return 1;
}

int builtin::game_bot::builtin_state(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    lua->pushstring(fb::model::enum_value::enum_tostring(bot->state()));
    return 1;
}

int builtin::game_bot::builtin_position(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    push_point(lua, bot->position());
    return 1;
}

int builtin::game_bot::builtin_exp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(bot->exp()));
    return 1;
}

int builtin::game_bot::builtin_map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(bot->map()));
    return 1;
}

int builtin::game_bot::builtin_item_slot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name = lua->tostring(2);
    lua->pushinteger(bot->get_item_slot_by_name(name));
    return 1;
}

int builtin::game_bot::builtin_item_count(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name = lua->tostring(2);
    lua->pushinteger(bot->get_item_count_by_name(name));
    return 1;
}

int builtin::game_bot::builtin_spell_slot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name = lua->tostring(2);
    lua->pushinteger(bot->get_spell_slot_by_name(name));
    return 1;
}

int builtin::game_bot::builtin_has_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto slot = require_integer<uint8_t>(L, lua, 2);
    lua->pushboolean(bot->get_item(slot).has_value());
    return 1;
}

int builtin::game_bot::builtin_has_spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto slot = require_integer<uint8_t>(L, lua, 2);
    lua->pushboolean(bot->get_spell(slot).has_value());
    return 1;
}

int builtin::game_bot::builtin_inventory_size(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(bot->items().size()));
    return 1;
}

int builtin::game_bot::builtin_has_item_by_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name = lua->tostring(2);
    lua->pushboolean(bot->has_item_by_name(name));
    return 1;
}

int builtin::game_bot::builtin_item_base_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto                           name = lua->tostring(2);
    fb::bot::game_bot::simple_item item(name, 0);
    lua->pushinteger(
        static_cast<lua_Integer>(item.get_price(static_cast<fb::bot::game_bot_controller&>(bot->controller))));
    return 1;
}

int builtin::game_bot::builtin_remove_buffs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    bot->remove_buffs();
    return 0;
}

int builtin::game_bot::builtin_has_buff(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name = lua->tostring(2);
    lua->pushboolean(bot->has_buff(name));
    return 1;
}

int builtin::game_bot::builtin_money(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->money()));
        return 1;
    }

    auto value   = require_integer<uint32_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_money(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_level(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->level()));
        return 1;
    }

    auto value   = require_integer<uint8_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_level(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->hp()));
        return 1;
    }

    auto value   = require_integer<uint32_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_hp(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_crowd_control(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(bot->crowd_control()));
    return 1;
}

int builtin::game_bot::builtin_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->mp()));
        return 1;
    }

    auto value   = require_integer<uint32_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_mp(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->base_hp()));
        return 1;
    }

    auto value   = require_integer<uint32_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_base_hp(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->base_mp()));
        return 1;
    }

    auto value   = require_integer<uint32_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_base_mp(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->strength()));
        return 1;
    }

    auto value   = require_integer<uint8_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_str(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->dexterity()));
        return 1;
    }

    auto value   = require_integer<uint8_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_dex(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushinteger(static_cast<lua_Integer>(bot->intelligence()));
        return 1;
    }

    auto value   = require_integer<uint8_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, value]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_int(value, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_gender(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    if (lua->argc() == 1)
    {
        lua->pushstring(fb::model::enum_value::enum_tostring(bot->gender()));
        return 1;
    }

    auto gender  = enum_parse<GENDER>(require_string(L, lua, 2));
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, gender]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_gender(gender, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

// ─── Async actions ────────────────────────────────────────────────────────────

int builtin::game_bot::builtin_request(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto* response_entry = lua_protocol::to_response_token(L, 2);
    auto  request        = lua_protocol::to_request(L, 3);
    if (lua->argc() < 4 || lua->is_function(4) == false)
        return luaL_error(L, "request(response, packet, validator) requires a function as third argument");

    if (auto* message = unified_response_opcode_error(response_entry->opcode))
        return luaL_error(L, "%s", message);

    lua_pushvalue(L, 4);
    auto validator_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto timeout = fb::model::timespan(INTEGRATION_DEFAULT_TIMEOUT);
    if (lua->argc() >= 5 && lua->is_nil(5) == false)
        timeout = fb::model::timespan(std::chrono::milliseconds(static_cast<int>(require_integer<int>(L, lua, 5))));

    auto bot_ptr = bot;
    auto result  = std::make_shared<std::shared_ptr<fb::protocol::header>>();

    auto builder  = lua->new_co_builder();
    builder.yield = [lua, bot_ptr, response_entry, request, validator_ref, timeout, result]() -> async::task<void> {
        auto* L_state = static_cast<lua_State*>(*lua);

        if (bot_ptr == nullptr)
        {
            luaL_unref(L_state, LUA_REGISTRYINDEX, validator_ref);
            co_return;
        }

        if (response_entry->ensure_registered != nullptr)
            response_entry->ensure_registered(static_cast<game_bot_controller&>(bot_ptr->controller));

        auto condition = [lua, validator_ref, response_entry](const fb::protocol::header& header) -> bool {
            auto* L_state = static_cast<lua_State*>(*lua);
            auto* lua_ctx = fb::lua::get(L_state);
            if (lua_ctx == nullptr)
                return false;

            response_entry->marshal_lua(L_state, header);

            lua_rawgeti(L_state, LUA_REGISTRYINDEX, validator_ref);
            lua_pushvalue(L_state, -2);
            if (lua_pcall(L_state, 1, 1, 0) != LUA_OK)
            {
                auto err = lua_ctx->tostring(-1);
                fb::logger::fatal("request validator failed: {}", err.empty() ? "unknown error" : err);
                lua_ctx->pop(2);
                return false;
            }

            auto accepted = lua_ctx->toboolean(-1);
            lua_ctx->pop(2);
            return accepted;
        };

        std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone;
        if (response_entry->clone != nullptr)
            clone = response_entry->clone;

        *result = co_await bot_ptr->request_by_opcode(bot_ptr,
                                                      response_entry->opcode,
                                                      *request,
                                                      condition,
                                                      timeout,
                                                      true,
                                                      true,
                                                      std::move(clone));

        luaL_unref(L_state, LUA_REGISTRYINDEX, validator_ref);
    };
    builder.resume = [lua, result, response_entry]() -> async::task<int> {
        if (*result == nullptr)
        {
            lua->pushboolean(false);
            co_return 1;
        }
        response_entry->marshal_lua(static_cast<lua_State*>(*lua), **result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_request_on(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto sender = lua->touserdata<fb::bot::game_bot>(1);
    if (sender == nullptr)
        return 0;

    auto listener = lua->touserdata<fb::bot::game_bot>(2);
    if (listener == nullptr)
        return 0;

    auto* response_entry = lua_protocol::to_response_token(L, 3);
    auto  request        = lua_protocol::to_request(L, 4);
    if (lua->argc() < 5 || lua->is_function(5) == false)
        return luaL_error(L, "request_on(target, response, packet, validator) requires a function as fourth argument");

    if (auto* message = unified_response_opcode_error(response_entry->opcode))
        return luaL_error(L, "%s", message);

    lua_pushvalue(L, 5);
    auto validator_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto timeout = fb::model::timespan(INTEGRATION_DEFAULT_TIMEOUT);
    if (lua->argc() >= 6 && lua->is_nil(6) == false)
        timeout = fb::model::timespan(std::chrono::milliseconds(static_cast<int>(require_integer<int>(L, lua, 6))));

    auto sender_ptr   = sender;
    auto listener_ptr = listener;
    auto result       = std::make_shared<std::shared_ptr<fb::protocol::header>>();

    auto builder  = lua->new_co_builder();
    builder.yield = [lua, sender_ptr, listener_ptr, response_entry, request, validator_ref, timeout, result]()
        -> async::task<void> {
        auto* L_state = static_cast<lua_State*>(*lua);

        if (sender_ptr == nullptr || listener_ptr == nullptr)
        {
            luaL_unref(L_state, LUA_REGISTRYINDEX, validator_ref);
            co_return;
        }

        if (response_entry->ensure_registered != nullptr)
            response_entry->ensure_registered(static_cast<game_bot_controller&>(listener_ptr->controller));

        auto condition = [lua, validator_ref, response_entry](const fb::protocol::header& header) -> bool {
            auto* L_state = static_cast<lua_State*>(*lua);
            auto* lua_ctx = fb::lua::get(L_state);
            if (lua_ctx == nullptr)
                return false;

            response_entry->marshal_lua(L_state, header);

            lua_rawgeti(L_state, LUA_REGISTRYINDEX, validator_ref);
            lua_pushvalue(L_state, -2);
            if (lua_pcall(L_state, 1, 1, 0) != LUA_OK)
            {
                auto err = lua_ctx->tostring(-1);
                fb::logger::fatal("request_on validator failed: {}", err.empty() ? "unknown error" : err);
                lua_ctx->pop(2);
                return false;
            }

            auto accepted = lua_ctx->toboolean(-1);
            lua_ctx->pop(2);
            return accepted;
        };

        std::function<std::shared_ptr<fb::protocol::header>(const fb::protocol::header&)> clone;
        if (response_entry->clone != nullptr)
            clone = response_entry->clone;

        *result = co_await sender_ptr->request_by_opcode(listener_ptr,
                                                         response_entry->opcode,
                                                         *request,
                                                         condition,
                                                         timeout,
                                                         true,
                                                         true,
                                                         std::move(clone));

        luaL_unref(L_state, LUA_REGISTRYINDEX, validator_ref);
    };
    builder.resume = [lua, result, response_entry]() -> async::task<int> {
        if (*result == nullptr)
        {
            lua->pushboolean(false);
            co_return 1;
        }
        response_entry->marshal_lua(static_cast<lua_State*>(*lua), **result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_send(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto request = lua_protocol::to_request(L, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, request]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;

        std::ignore = co_await bot_ptr->send(*request);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_chat(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto text    = lua->tostring(2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, text]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        bot_ptr->chat(text);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_move(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto direction = enum_parse<DIRECTION>(require_string(L, lua, 2));
    auto step      = static_cast<int>(opt_integer(lua, 3, 1));
    auto bot_ptr   = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, direction, step]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->move(direction, step);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_direction(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto direction = enum_parse<DIRECTION>(require_string(L, lua, 2));
    auto bot_ptr   = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, direction]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->direction(direction, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_map_move(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto map     = lua->tostring(2);
    auto x       = require_integer<uint16_t>(L, lua, 3);
    auto y       = require_integer<uint16_t>(L, lua, 4);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, map, x, y]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->map_move(map, x, y, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_transfer(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto request = lua_protocol::to_request(L, 2);
    auto bot_ptr = bot;

    auto weak_slot = static_cast<std::weak_ptr<fb::bot::game_bot>*>(lua_touserdata(*lua, 1));
    if (weak_slot == nullptr)
        return 0;

    auto result = std::make_shared<std::shared_ptr<fb::bot::game_bot>>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, request, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;

        *result = co_await bot_ptr->transfer(*request, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [weak_slot, result]() -> async::task<int> {
        if (*result == nullptr)
            co_return 0;

        if (weak_slot == nullptr)
        {
            fb::logger::warn("builtin transfer: userdata not updated (null weak_slot)");
            co_return 0;
        }

        weak_slot->~weak_ptr<fb::bot::game_bot>();
        new (weak_slot) std::weak_ptr<fb::bot::game_bot>(*result);
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_create_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto count   = static_cast<uint32_t>(opt_integer(lua, 3, 1));
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name, count]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->create_item(name, count, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_equip(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto slot    = require_integer<uint8_t>(L, lua, 2);
    auto bot_ptr = bot;
    auto result  = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, slot, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->equip(slot, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_unequip(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto parts   = enum_parse<EQUIPMENT_PARTS>(require_string(L, lua, 2));
    auto bot_ptr = bot;
    auto result  = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, parts, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->unequip(parts, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_clear_inventory(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->clear_inventory(INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_learn_spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto bot_ptr = bot;
    auto result  = std::make_shared<int>(-1);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->learn_spell(name, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushinteger(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_learn_spells(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto names   = parse_string_list(lua, 2);
    auto bot_ptr = bot;
    auto result  = std::make_shared<size_t>(0);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, names, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->learn_spells(names, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushinteger(static_cast<lua_Integer>(*result));
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_clear_all_spells(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->clear_all_spells(INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_setup_bot_stats(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto max_hp = require_integer<int>(L, lua, 2);
    auto max_mp = require_integer<int>(L, lua, 3);

    std::optional<int> current_hp;
    std::optional<int> current_mp;
    if (lua->argc() >= 4 && lua->is_nil(4) == false)
        current_hp = require_integer<int>(L, lua, 4);
    if (lua->argc() >= 5 && lua->is_nil(5) == false)
        current_mp = require_integer<int>(L, lua, 5);

    auto bot_ptr = bot;
    auto result  = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, max_hp, max_mp, current_hp, current_mp, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result =
            co_await bot_ptr->setup_bot_stats(max_hp, max_mp, current_hp, current_mp, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_set_current_hp_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto current_hp = require_integer<int>(L, lua, 2);
    auto current_mp = require_integer<int>(L, lua, 3);
    auto bot_ptr    = bot;
    auto result     = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, current_hp, current_mp, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->set_current_hp_mp(current_hp, current_mp, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_set_max_hp_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto max_hp  = require_integer<int>(L, lua, 2);
    auto max_mp  = require_integer<int>(L, lua, 3);
    auto bot_ptr = bot;
    auto result  = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, max_hp, max_mp, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->set_max_hp_mp(max_hp, max_mp, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_spawn_monster(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto x       = require_integer<uint16_t>(L, lua, 3);
    auto y       = require_integer<uint16_t>(L, lua, 4);
    auto bot_ptr = bot;
    auto result  = std::make_shared<fb::bot::spawned_monster_info>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name, x, y, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->spawn_monster(name, x, y, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        push_spawned_monster(lua, *result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_spawn_monster_relative(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name       = lua->tostring(2);
    auto relative_x = require_integer<int>(L, lua, 3);
    auto relative_y = require_integer<int>(L, lua, 4);
    auto bot_ptr    = bot;
    auto result     = std::make_shared<fb::bot::spawned_monster_info>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name, relative_x, relative_y, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->spawn_monster_relative(name, relative_x, relative_y, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        push_spawned_monster(lua, *result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_spawn_monsters_bulk(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto range   = require_integer<uint8_t>(L, lua, 3);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name, range]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->spawn_monsters_bulk(name, range, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_spawn_monsters_relative(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name      = lua->tostring(2);
    auto positions = parse_relative_positions(lua, 3);
    auto bot_ptr   = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name, positions]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        std::ignore = co_await bot_ptr->spawn_monsters_relative(name, positions, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_move_back(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto point    = parse_point(lua, 2);
    auto interval = static_cast<int>(opt_integer(lua, 3, 100));
    auto bot_ptr  = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, point, interval]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->move_bot_back_to_position(point,
                                                    std::chrono::milliseconds(interval),
                                                    INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_sleep(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto ms      = require_integer<int>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, ms]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->sleep(std::chrono::milliseconds(ms));
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_drop_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto index   = require_integer<uint8_t>(L, lua, 2);
    auto all     = lua->toboolean(3);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, index, all]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->drop_item(index, all, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_drop_money(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto amount  = require_integer<uint32_t>(L, lua, 2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, amount]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->drop_money(amount, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_change_class(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->change_class(name, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_apply_item_condition(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        auto* item = find_item_by_name(name);
        if (item == nullptr)
            throw std::runtime_error(std::format("item not found: {}", name));
        co_await bot_ptr->apply_condition(item->condition, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_reverse_item_condition(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        auto* item = find_item_by_name(name);
        if (item == nullptr)
            throw std::runtime_error(std::format("item not found: {}", name));
        co_await bot_ptr->reverse_condition(item->condition, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_invite_group(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto caster = lua->touserdata<fb::bot::game_bot>(1);
    if (caster == nullptr)
        return 0;

    auto target = lua->touserdata<fb::bot::game_bot>(2);
    if (target == nullptr)
        return 0;

    auto result = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [caster, target, result]() -> async::task<void> {
        if (caster == nullptr || target == nullptr)
            co_return;
        *result = co_await caster->invite_group(target, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_leave_group(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto bot_ptr = bot;
    auto result  = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        *result = co_await bot_ptr->leave_group(INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_kick_group(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto caster = lua->touserdata<fb::bot::game_bot>(1);
    if (caster == nullptr)
        return 0;

    auto target = lua->touserdata<fb::bot::game_bot>(2);
    if (target == nullptr)
        return 0;

    auto result = std::make_shared<bool>(false);

    auto builder  = lua->new_co_builder();
    builder.yield = [caster, target, result]() -> async::task<void> {
        if (caster == nullptr || target == nullptr)
            co_return;
        *result = co_await caster->kick_group(target, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_fill_inventory(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->fill_inventory(name, INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::game_bot::builtin_create_npc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto name    = lua->tostring(2);
    auto bot_ptr = bot;

    struct npc_info
    {
        uint32_t    oid;
        std::string name;
    };
    auto result = std::make_shared<npc_info>();

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr, name, result]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        auto npc     = co_await bot_ptr->create_npc(name, INTEGRATION_DEFAULT_TIMEOUT);
        result->oid  = npc.oid;
        result->name = npc.name;
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        lua->pushstring("oid");
        lua->pushinteger(result->oid);
        lua->settable(-3);
        lua->pushstring("name");
        lua->pushstring(result->name);
        lua->settable(-3);
        co_return 1;
    };
    return builder.run();
}

int builtin::game_bot::builtin_clear_all_drop_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto bot_ptr = bot;

    auto builder  = lua->new_co_builder();
    builder.yield = [bot_ptr]() -> async::task<void> {
        if (bot_ptr == nullptr)
            co_return;
        co_await bot_ptr->clear_all_drop_items(INTEGRATION_DEFAULT_TIMEOUT);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}
