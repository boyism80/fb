#include <fb/bot/brain/brain.h>
#include <fb/bot/builtin/brain.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>
#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/game/inventory.h>
#include <fb/lua.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <string>
#include <unordered_set>
#include <vector>

using namespace fb::bot;
using namespace std::chrono_literals;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

constexpr auto DIALOG_TIMEOUT = 3s;
constexpr auto LOOT_TIMEOUT   = 3s;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::bot::brain, "fb.bot.brain")
    {"state",    builtin::brain::builtin_state},
    {"states",   builtin::brain::builtin_states},
    {"progress", builtin::brain::builtin_progress},
    {"npc",      builtin::brain::builtin_npc},
    {"persona",  builtin::brain::builtin_persona},
    {"active",   builtin::brain::builtin_active},
    {"observe",  builtin::brain::builtin_observe},
    {"route",    builtin::brain::builtin_route},
    {"walk",     builtin::brain::builtin_walk},
    {"remember", builtin::brain::builtin_remember},
    {"npcs",     builtin::brain::builtin_npcs},
    {"ask",      builtin::brain::builtin_ask},
    {"death",    builtin::brain::builtin_death},
    {"bury",     builtin::brain::builtin_bury},
    {"inventory", builtin::brain::builtin_inventory},
    {"loot",     builtin::brain::builtin_loot},
    {"watch",    builtin::brain::builtin_watch},
    {"pick",     builtin::brain::builtin_pick},
    {"items",    builtin::brain::builtin_items},
    {"concern",  builtin::brain::builtin_concern},
    {"activities", builtin::brain::builtin_activities},
    {"failure",  builtin::brain::builtin_failure},
    {"expect",   builtin::brain::builtin_expect},
    {"worn",     builtin::brain::builtin_worn},
    {"item",     builtin::brain::builtin_item},
    {"reserve",  builtin::brain::builtin_reserve},
    {"release",  builtin::brain::builtin_release},
    {"reservations", builtin::brain::builtin_reservations},
    {"recall",   builtin::brain::builtin_recall},
    {"mobs",     builtin::brain::builtin_mobs},
    {"spawns",   builtin::brain::builtin_spawns},
    {"attack",   builtin::brain::builtin_attack},
END_LUA_EXTENSION; // clang-format on

int builtin::brain::builtin_state(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto mind = brain->find(bot->id);
    if (mind.has_value() == false || (mind->history.empty() && mind->failures == 0))
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();
    lua->pushstring("name");
    lua->pushstring(fb::bot::brain::name(mind->current.state));
    lua->settable(-3);
    lua->pushstring("role");
    lua->pushstring(fb::bot::brain::name(mind->role));
    lua->settable(-3);
    lua->pushstring("since");
    lua_pushnumber(L, fb::bot::brain::seconds(mind->since));
    lua->settable(-3);
    lua->pushstring("reason");
    lua->pushstring(mind->current.reason);
    lua->settable(-3);
    lua->pushstring("failures");
    lua->pushinteger(mind->failures);
    lua->settable(-3);
    return 1;
}

int builtin::brain::builtin_states(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto since = lua->argc() >= 3 && lua_type(L, 3) == LUA_TNUMBER ? lua_tonumber(L, 3) : 0.0;
    auto mind  = brain->find(bot->id);
    auto index = 1;
    lua->new_table();
    for (auto& transition : mind.has_value() ? mind->history : std::vector<fb::bot::brain::transition>{})
    {
        auto at = fb::bot::brain::seconds(transition.at);
        if (at < since)
            continue;

        lua->pushinteger(index++);
        lua->new_table();
        lua->pushstring("name");
        lua->pushstring(fb::bot::brain::name(transition.to.state));
        lua->settable(-3);
        lua->pushstring("task");
        lua->pushstring(transition.to.task);
        lua->settable(-3);
        lua->pushstring("step");
        lua->pushinteger(transition.to.step);
        lua->settable(-3);
        lua->pushstring("reason");
        lua->pushstring(transition.to.reason);
        lua->settable(-3);
        lua->pushstring("at");
        lua_pushnumber(L, at);
        lua->settable(-3);
        lua->settable(-3);
    }
    return 1;
}

int builtin::brain::builtin_progress(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto mind = brain->find(bot->id);
    if (mind.has_value() == false || mind->history.empty())
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();
    lua->pushstring("task");
    lua->pushstring(mind->current.task);
    lua->settable(-3);
    lua->pushstring("step");
    lua->pushinteger(mind->current.step);
    lua->settable(-3);
    return 1;
}

int builtin::brain::builtin_npc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto found = fb::bot::brain::npc(*bot, lua->tostring(3));
    if (found.has_value() == false)
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();
    lua->pushstring("oid");
    lua->pushinteger(found->oid);
    lua->settable(-3);
    lua->pushstring("x");
    lua->pushinteger(found->position.x);
    lua->settable(-3);
    lua->pushstring("y");
    lua->pushinteger(found->position.y);
    lua->settable(-3);
    return 1;
}

int builtin::brain::builtin_persona(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto mind = brain->find(bot->id);
    lua->new_table();
    for (auto& [key, value] : mind.has_value() ? mind->traits.persona : std::unordered_map<std::string, double>{})
    {
        lua->pushstring(key);
        lua_pushnumber(L, value);
        lua->settable(-3);
    }
    return 1;
}

int builtin::brain::builtin_active(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    lua->pushboolean(brain->active(bot->id));
    return 1;
}

int builtin::brain::builtin_observe(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto seen = fb::bot::brain::observe(*bot);
    auto mind = brain->find(bot->id);
    lua->new_table();
    lua->pushstring("level");
    lua->pushinteger(seen.level);
    lua->settable(-3);
    lua->pushstring("class");
    lua->pushstring(fb::model::enum_value::enum_tostring(seen.cls));
    lua->settable(-3);
    lua->pushstring("promotion");
    lua->pushinteger(seen.promotion);
    lua->settable(-3);
    lua->pushstring("ghost");
    lua->pushboolean(seen.ghost);
    lua->settable(-3);
    lua->pushstring("map");
    lua->pushinteger(seen.map);
    lua->settable(-3);
    lua->pushstring("nation");
    lua->pushinteger(seen.nation);
    lua->settable(-3);
    lua->pushstring("online");
    lua->pushboolean(bot->aborted() == false);
    lua->settable(-3);
    lua->pushstring("bag");
    lua->pushinteger(static_cast<lua_Integer>(bot->items().size()));
    lua->settable(-3);
    lua->pushstring("capacity");
    lua->pushinteger(fb::game::CONTAINER_CAPACITY);
    lua->settable(-3);
    if (mind.has_value() && mind->jumped.empty() == false)
    {
        lua->pushstring("jumped");
        lua->pushstring(mind->jumped);
        lua->settable(-3);
    }
    lua->pushstring("hp");
    lua->pushinteger(bot->hp());
    lua->settable(-3);
    lua->pushstring("max_hp");
    lua->pushinteger(bot->base_hp());
    lua->settable(-3);
    lua->pushstring("regen");
    lua->pushinteger(std::chrono::duration_cast<std::chrono::milliseconds>(
                         static_cast<std::chrono::microseconds>(fb::model::const_value::recovery::hp_interval))
                         .count());
    lua->settable(-3);
    lua->pushstring("pace");
    lua->new_table();
    for (auto& [action, pace] :
         mind.has_value() ? mind->paces : std::unordered_map<std::string, fb::bot::brain::pace>{})
    {
        lua->pushstring(action);
        lua->pushinteger(pace.total.count() / std::max<uint32_t>(pace.count, 1u));
        lua->settable(-3);
    }
    lua->settable(-3);
    return 1;
}

fb::bot::brain::destination builtin::brain::read_destination(lua_State* L, int index)
{
    auto lua    = fb::lua::get(L);
    auto result = fb::bot::brain::destination{};
    index       = lua_absindex(L, index);
    if (lua_istable(L, index) == false)
        return result;

    lua_getfield(L, index, "names");
    if (lua_istable(L, -1))
    {
        auto names = std::unordered_set<std::string>{};
        for (auto i = lua_Integer{1}; lua_rawgeti(L, -1, i) == LUA_TSTRING; i++)
        {
            names.insert(lua->tostring(-1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        auto maps = fb::model::table::map;
        for (auto& [id, model] : maps)
        {
            if (names.contains(model.name))
                result.maps.insert(static_cast<uint16_t>(id));
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "maps");
    if (lua_istable(L, -1))
    {
        for (auto i = lua_Integer{1}; lua_rawgeti(L, -1, i) == LUA_TNUMBER; i++)
        {
            result.maps.insert(static_cast<uint16_t>(lua_tointeger(L, -1)));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "map");
    if (lua_isinteger(L, -1))
    {
        result.maps.insert(static_cast<uint16_t>(lua_tointeger(L, -1)));
    }
    else if (lua_isstring(L, -1))
    {
        auto model = fb::model::table::map->name2map(lua->tostring(-1));
        if (model != nullptr)
            result.maps.insert(static_cast<uint16_t>(model->id));
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "x");
    lua_getfield(L, index, "y");
    if (lua_isinteger(L, -2) && lua_isinteger(L, -1))
        result.at =
            atlas::position(static_cast<uint16_t>(lua_tointeger(L, -2)), static_cast<uint16_t>(lua_tointeger(L, -1)));
    lua_pop(L, 2);

    lua_getfield(L, index, "range");
    result.range = static_cast<uint16_t>(lua_tointeger(L, -1));
    lua_pop(L, 1);
    return result;
}

fb::bot::brain::detour builtin::brain::read_detour(lua_State* L, int index)
{
    auto lua    = fb::lua::get(L);
    auto result = fb::bot::brain::detour{};
    index       = lua_absindex(L, index);
    if (lua_istable(L, index) == false)
        return result;

    for (auto [field, keys] : {
             std::pair{"closed", &result.closed},
             std::pair{"avoid",  &result.avoid }
    })
    {
        lua_getfield(L, index, field);
        if (lua_istable(L, -1))
        {
            for (auto i = lua_Integer{1}; lua_rawgeti(L, -1, i) == LUA_TTABLE; i++)
            {
                lua_getfield(L, -1, "map");
                lua_getfield(L, -2, "x");
                lua_getfield(L, -3, "y");
                keys->insert(atlas::gate(static_cast<uint16_t>(lua_tointeger(L, -3)),
                                         atlas::position(static_cast<uint16_t>(lua_tointeger(L, -2)),
                                                         static_cast<uint16_t>(lua_tointeger(L, -1)))));
                lua_pop(L, 4);
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }

    lua_getfield(L, index, "pace");
    result.pace = std::chrono::milliseconds(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, index, "rush");
    result.rush = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, index, "jumps");
    if (lua_istable(L, -1))
    {
        for (auto i = lua_Integer{1}; lua_rawgeti(L, -1, i) == LUA_TTABLE; i++)
        {
            auto jump = fb::bot::brain::jump{};
            lua_getfield(L, -1, "spell");
            if (lua_isstring(L, -1))
                jump.spell = lua->tostring(-1);
            lua_getfield(L, -2, "item");
            if (lua_isstring(L, -1))
                jump.item = lua->tostring(-1);
            lua_getfield(L, -3, "via");
            auto via          = lua_isstring(L, -1) ? lua->tostring(-1) : std::string{};
            jump.teleport.via = via == "CARDINAL" ? atlas::VIA::CARDINAL
                                : via == "REVIVE" ? atlas::VIA::REVIVE
                                : via == "RANDOM" ? atlas::VIA::RANDOM
                                                  : atlas::VIA::FIXED;
            lua_getfield(L, -4, "cost");
            jump.teleport.cost = static_cast<uint32_t>(lua_tointeger(L, -1));
            lua_pop(L, 4);

            lua_getfield(L, -1, "to");
            if (lua_istable(L, -1))
            {
                for (auto j = lua_Integer{1}; lua_rawgeti(L, -1, j) != LUA_TNIL; j++)
                {
                    auto model = lua_isinteger(L, -1)
                                     ? fb::model::table::map->find(static_cast<uint32_t>(lua_tointeger(L, -1)))
                                     : fb::model::table::map->name2map(lua->tostring(-1));
                    if (model != nullptr)
                        jump.teleport.to.push_back(static_cast<uint16_t>(model->id));
                    lua_pop(L, 1);
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);

            lua_getfield(L, -1, "inputs");
            if (lua_istable(L, -1))
            {
                for (auto side :
                     {atlas::CARDINAL::EAST, atlas::CARDINAL::WEST, atlas::CARDINAL::SOUTH, atlas::CARDINAL::NORTH})
                {
                    lua_getfield(L, -1, fb::model::enum_value::enum_tostring(side));
                    if (lua_isstring(L, -1))
                        jump.inputs[side] = lua->tostring(-1);
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);

            result.jumps.push_back(std::move(jump));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return result;
}

int builtin::brain::builtin_route(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto destination = builtin::brain::read_destination(L, 3);
    auto detour      = builtin::brain::read_detour(L, 4);
    auto result      = std::make_shared<std::optional<atlas::plan>>();

    auto builder  = lua->new_co_builder();
    builder.yield = [brain, bot, destination, detour, result]() -> async::task<void> {
        auto map   = bot->map();
        auto here  = bot->position();
        auto level = bot->level();
        *result = co_await brain->offload<std::optional<atlas::plan>>(bot, [map, here, destination, level, detour]() {
            return atlas::get().route(map, here, destination.maps, level, detour.closed);
        });
    };
    builder.resume = [lua, result]() -> async::task<int> {
        if (result->has_value() == false)
        {
            lua->pushnil();
            co_return 1;
        }

        lua->new_table();
        lua->pushstring("dest");
        lua->pushinteger(result->value().dest);
        lua->settable(-3);
        lua->pushstring("cost");
        lua->pushinteger(result->value().cost);
        lua->settable(-3);
        lua->pushstring("hops");
        lua->pushinteger(static_cast<lua_Integer>(result->value().hops.size()));
        lua->settable(-3);
        lua->pushstring("x");
        lua->pushinteger(result->value().arrival.x);
        lua->settable(-3);
        lua->pushstring("y");
        lua->pushinteger(result->value().arrival.y);
        lua->settable(-3);
        co_return 1;
    };
    return builder.run();
}

int builtin::brain::builtin_remember(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto key   = std::string(lua_tostring(L, 3));
    auto value = lua_isnoneornil(L, 4) ? std::optional<fb::lua::value>{} : fb::lua::value::copy_data(L, 4);
    brain->remember(bot->id, key, value);
    return 0;
}

int builtin::brain::builtin_npcs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto names = std::unordered_set<std::string>{};
    if (lua_istable(L, 3))
    {
        for (auto i = lua_Integer{1}; lua_rawgeti(L, 3, i) == LUA_TSTRING; i++)
        {
            names.insert(lua->tostring(-1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }

    auto npcs   = fb::model::table::npc;
    auto spawns = fb::model::table::npc_spawn;
    auto index  = lua_Integer{1};
    lua->new_table();
    for (auto& [map, entries] : spawns)
    {
        for (auto& entry : entries)
        {
            auto model = npcs->find(entry.npc);
            if (model == nullptr || names.contains(model->name) == false)
                continue;

            lua->new_table();
            lua->pushstring("name");
            lua->pushstring(model->name);
            lua->settable(-3);
            lua->pushstring("map");
            lua->pushinteger(map);
            lua->settable(-3);
            lua->pushstring("x");
            lua->pushinteger(entry.position.x);
            lua->settable(-3);
            lua->pushstring("y");
            lua->pushinteger(entry.position.y);
            lua->settable(-3);
            lua_rawseti(L, -2, index++);
        }
    }
    return 1;
}

int builtin::brain::builtin_ask(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto request = fb::bot::integration::lua_protocol::to_request(L, 3, bot->client_version());
    if (request == nullptr)
        return 0;

    auto answered = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder();
    builder.yield = [bot, request, answered]() -> async::task<void> {
        try
        {
            std::ignore = co_await bot->first_of(
                {fb::bot::integration::dialog_bot::opcode, fb::bot::integration::dialog_ext_bot::opcode},
                *request,
                DIALOG_TIMEOUT);
            *answered = true;
        }
        catch (const std::exception&)
        { }
    };
    builder.resume = [lua, answered]() -> async::task<int> {
        lua->pushboolean(*answered);
        co_return 1;
    };
    return builder.run();
}

int builtin::brain::builtin_death(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto mind = brain->find(bot->id);
    if (mind.has_value() == false || mind->died.has_value() == false)
    {
        lua->pushnil();
        return 1;
    }

    auto& died     = mind->died.value();
    auto  age      = std::chrono::duration<double>(fb::bot::brain::clock::now() - died.when).count();
    auto  lifetime = std::chrono::duration<double>(
                        static_cast<std::chrono::microseconds>(fb::model::const_value::time::GROUND_ITEM) +
                        static_cast<std::chrono::microseconds>(fb::model::const_value::death_penalty::warmth_time))
                        .count();
    lua->new_table();
    lua->pushstring("map");
    lua->pushinteger(died.map);
    lua->settable(-3);
    lua->pushstring("x");
    lua->pushinteger(died.at.x);
    lua->settable(-3);
    lua->pushstring("y");
    lua->pushinteger(died.at.y);
    lua->settable(-3);
    lua->pushstring("age");
    lua_pushnumber(L, age);
    lua->settable(-3);
    lua->pushstring("left");
    lua_pushnumber(L, lifetime - age);
    lua->settable(-3);
    lua->pushstring("money");
    lua->pushinteger(died.money);
    lua->settable(-3);
    lua->pushstring("items");
    lua->new_table();
    for (auto& [name, count] : died.items)
    {
        lua->pushstring(name);
        lua->pushinteger(count);
        lua->settable(-3);
    }
    lua->settable(-3);
    return 1;
}

int builtin::brain::builtin_bury(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    brain->bury(*bot);
    return 0;
}

int builtin::brain::builtin_inventory(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto models = fb::model::table::item;
    auto index  = lua_Integer{1};
    lua->new_table();
    for (auto& [slot, item] : bot->items())
    {
        auto model = models->name2item(item.name);
        lua->new_table();
        lua->pushstring("slot");
        lua->pushinteger(slot);
        lua->settable(-3);
        lua->pushstring("name");
        lua->pushstring(item.name);
        lua->settable(-3);
        lua->pushstring("count");
        lua->pushinteger(item.count);
        lua->settable(-3);
        lua->pushstring("type");
        lua->pushstring(model != nullptr ? fb::model::enum_value::enum_tostring(model->type) : "");
        lua->settable(-3);
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

int builtin::brain::builtin_loot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto picked   = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder();
    builder.yield = [bot, picked]() -> async::task<void> {
        try
        {
            constexpr auto ITEM    = game_resp::item_update<fb::protocol::CLIENT_VERSION::v550>::opcode;
            constexpr auto MESSAGE = game_resp::message::opcode;
            auto           money   = bot->money();
            auto           refusal =
                std::unordered_set<std::string>{fb::model::const_value::string::MESSAGE_ITEM_FULL,
                                                fb::model::const_value::string::MESSAGE_ITEM_DEATH_PENALTY_WARMTH};
            auto answer = co_await bot->first_of(
                {ITEM, game_resp::update<fb::protocol::CLIENT_VERSION::v550>::opcode, MESSAGE},
                bot->make<game_reqs::loot>(true).get(),
                [bot, money, refusal](uint8_t opcode, const fb::protocol::header& header) {
                    if (opcode == MESSAGE)
                        return refusal.contains(static_cast<const game_resp::message&>(header).text);
                    else
                        return opcode == ITEM || bot->money() != money;
                },
                LOOT_TIMEOUT);
            *picked = answer.first != MESSAGE;
        }
        catch (const std::exception&)
        { }
    };
    builder.resume = [lua, picked]() -> async::task<int> {
        lua->pushboolean(*picked);
        co_return 1;
    };
    return builder.run();
}

int builtin::brain::builtin_watch(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto oid      = static_cast<uint32_t>(lua_tointeger(L, 3));
    auto timeout  = std::chrono::milliseconds(lua_tointeger(L, 4));
    auto changed  = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder();
    builder.yield = [brain, bot, oid, timeout, changed]() -> async::task<void> {
        try
        {
            std::ignore = co_await bot->first_of(
                {game_resp::move::opcode, game_resp::hide::opcode, game_resp::die::opcode},
                nullptr,
                [oid](uint8_t opcode, const fb::protocol::header& header) {
                    if (opcode == game_resp::move::opcode)
                        return static_cast<const game_resp::move&>(header).id == oid;
                    else if (opcode == game_resp::hide::opcode)
                        return static_cast<const game_resp::hide&>(header).oid == oid;
                    else
                        return static_cast<const game_resp::die&>(header).oid == oid;
                },
                timeout);
            *changed = true;
        }
        catch (const std::exception&)
        { }
        brain->progress(bot->id);
    };
    builder.resume = [lua, changed]() -> async::task<int> {
        lua->pushboolean(*changed);
        co_return 1;
    };
    return builder.run();
}

int builtin::brain::builtin_activities(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto mind  = brain->find(bot->id);
    auto index = lua_Integer{1};
    lua->new_table();
    for (auto& activity : mind.has_value() ? mind->activities : std::vector<std::string>{})
    {
        lua->pushstring(activity);
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

int builtin::brain::builtin_failure(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto mind = brain->find(bot->id);
    auto task = std::string(lua_tostring(L, 3));
    if (mind.has_value() == false || mind->records.contains(task) == false)
    {
        lua->pushnil();
        return 1;
    }

    auto& entry = mind->records.at(task);
    lua->new_table();
    lua->pushstring("failures");
    lua->pushinteger(entry.failures);
    lua->settable(-3);
    lua->pushstring("reason");
    lua->pushstring(entry.reason);
    lua->settable(-3);
    lua->pushstring("age");
    lua->pushinteger(
        std::chrono::duration_cast<std::chrono::milliseconds>(fb::bot::brain::clock::now() - entry.at).count());
    lua->settable(-3);
    lua->pushstring("duration");
    lua->pushinteger(entry.duration.count());
    lua->settable(-3);
    lua->pushstring("map");
    lua->pushinteger(entry.map);
    lua->settable(-3);
    lua->pushstring("level");
    lua->pushinteger(entry.level);
    lua->settable(-3);
    return 1;
}

int builtin::brain::builtin_expect(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    brain->expect(bot->id, std::chrono::milliseconds(lua_tointeger(L, 3)));
    return 0;
}

int builtin::brain::builtin_worn(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    using ITEM_TYPE = fb::model::enum_value::ITEM_TYPE;
    struct slot
    {
        ITEM_TYPE type;
        bool      dress;
        uint16_t  value;
        uint8_t   color;
    };

    auto slots = std::vector<slot>{
        {ITEM_TYPE::WEAPON, true, bot->weapon_dress(), bot->weapon_color()},
        {ITEM_TYPE::ARMOR,  true, bot->armor_dress(),  bot->armor_color() },
        {ITEM_TYPE::SHIELD, true, bot->shield_dress(), bot->shield_color()},
    };
    auto accessories =
        std::array{ITEM_TYPE::HELMET, ITEM_TYPE::RING, ITEM_TYPE::RING, ITEM_TYPE::AUXILIARY, ITEM_TYPE::AUXILIARY};
    for (auto i = size_t{0}; i < bot->accessories().size() && i < accessories.size(); i++)
    {
        auto& [look, color] = bot->accessories().at(i);
        slots.push_back(slot{accessories.at(i), false, look, color});
    }

    auto models = fb::model::table::item;
    auto index  = lua_Integer{1};
    lua->new_table();
    for (auto& entry : slots)
    {
        lua->new_table();
        lua->pushstring("type");
        lua->pushstring(fb::model::enum_value::enum_tostring(entry.type));
        lua->settable(-3);
        lua->pushstring("names");
        lua->new_table();
        auto name  = lua_Integer{1};
        auto empty = entry.dress ? entry.value == 0 : entry.value == 0xFFFF;
        for (auto& [_, model] : models)
        {
            auto equipment = dynamic_cast<const fb::model::equipment*>(&model);
            if (empty || equipment == nullptr || model.type != entry.type)
                continue;

            auto matched = entry.dress ? equipment->dress == entry.value && model.color == entry.color
                                       : model.look - fb::bot::game_bot::sight::ITEM_LOOK == entry.value &&
                                             model.color == entry.color;
            if (matched == false)
                continue;

            lua->pushstring(model.name);
            lua_rawseti(L, -2, name++);
        }
        lua->settable(-3);
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

int builtin::brain::builtin_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = fb::model::table::item->name2item(lua->tostring(2));
    if (model == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();
    lua->pushstring("type");
    lua->pushstring(fb::model::enum_value::enum_tostring(model->type));
    lua->settable(-3);
    lua->pushstring("price");
    lua->pushinteger(model->price);
    lua->settable(-3);
    auto equipment = dynamic_cast<const fb::model::equipment*>(model);
    if (equipment != nullptr)
    {
        lua->pushstring("defense");
        lua->pushinteger(equipment->defensive_physical);
        lua->settable(-3);
        lua->pushstring("hp");
        lua->pushinteger(equipment->base_hp);
        lua->settable(-3);
        lua->pushstring("hit");
        lua->pushinteger(equipment->hit);
        lua->settable(-3);
        lua->pushstring("bonus");
        lua->pushinteger(equipment->damage);
        lua->settable(-3);
    }
    auto weapon = dynamic_cast<const fb::model::weapon*>(model);
    if (weapon != nullptr)
    {
        lua->pushstring("damage");
        lua_pushnumber(L, (weapon->damage_small.min + weapon->damage_small.max) / 2.0);
        lua->settable(-3);
    }
    return 1;
}

int builtin::brain::builtin_concern(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto value = fb::bot::brain::concern{};
    if (lua_istable(L, 3))
    {
        lua_getfield(L, 3, "attacks");
        if (lua_istable(L, -1))
        {
            lua_pushnil(L);
            while (lua_next(L, -2) != 0)
            {
                if (lua_type(L, -2) == LUA_TSTRING && lua_toboolean(L, -1))
                    value.attacks.insert(lua_tostring(L, -2));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        lua_getfield(L, 3, "hp");
        value.hp = static_cast<uint32_t>(lua_tointeger(L, -1));
        lua_pop(L, 1);
    }
    brain->watch(bot->id, value);
    return 0;
}

int builtin::brain::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto models = fb::model::table::item;
    auto index  = lua_Integer{1};
    lua->new_table();
    for (auto& [oid, entry] : bot->sight.entries())
    {
        if (entry.look < fb::bot::game_bot::sight::ITEM_LOOK)
            continue;

        lua->new_table();
        lua->pushstring("oid");
        lua->pushinteger(oid);
        lua->settable(-3);
        lua->pushstring("x");
        lua->pushinteger(entry.position.x);
        lua->settable(-3);
        lua->pushstring("y");
        lua->pushinteger(entry.position.y);
        lua->settable(-3);
        lua->pushstring("names");
        lua->new_table();
        auto name = lua_Integer{1};
        for (auto& [_, model] : models)
        {
            if (model.look != entry.look || model.color != entry.color)
                continue;

            lua->pushstring(model.name);
            lua_rawseti(L, -2, name++);
        }
        lua->settable(-3);
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

int builtin::brain::builtin_pick(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto weights = std::vector<std::pair<std::string, double>>{};
    if (lua_istable(L, 3))
    {
        lua_pushnil(L);
        while (lua_next(L, 3) != 0)
        {
            if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TNUMBER)
                weights.emplace_back(lua_tostring(L, -2), lua_tonumber(L, -1));
            lua_pop(L, 1);
        }
    }
    std::sort(weights.begin(), weights.end());

    auto picked = brain->pick(bot->id, weights);
    if (picked.has_value())
        lua_pushstring(L, picked->c_str());
    else
        lua->pushnil();
    return 1;
}

int builtin::brain::builtin_reserve(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    fb::bot::brain::reserve(bot->name(), lua_tostring(L, 3));
    return 0;
}

int builtin::brain::builtin_release(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    fb::bot::brain::release(bot->name());
    return 0;
}

int builtin::brain::builtin_reservations(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    lua->new_table();
    for (auto& [value, count] : fb::bot::brain::reservations(bot->name()))
    {
        lua_pushstring(L, value.c_str());
        lua->pushinteger(count);
        lua->settable(-3);
    }
    return 1;
}

int builtin::brain::builtin_recall(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto value = brain->recall(bot->id, std::string(lua_tostring(L, 3)));
    if (value.has_value())
        value->push(*lua);
    else
        lua->pushnil();
    return 1;
}

int builtin::brain::builtin_walk(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto destination = builtin::brain::read_destination(L, 3);
    auto detour      = builtin::brain::read_detour(L, 4);
    auto result      = std::make_shared<fb::bot::brain::event>();

    auto builder  = lua->new_co_builder();
    builder.yield = [brain, bot, destination, detour, result]() -> async::task<void> {
        brain->act(bot->id, "walk");
        *result = co_await brain->walk(bot, destination, detour);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        lua->pushstring("type");
        lua->pushstring(result->type);
        lua->settable(-3);
        lua->pushstring("detail");
        lua->pushstring(result->detail);
        lua->settable(-3);
        lua->pushstring("oid");
        lua->pushinteger(result->oid);
        lua->settable(-3);
        lua->pushstring("gx");
        lua->pushinteger(result->goal.x);
        lua->settable(-3);
        lua->pushstring("gy");
        lua->pushinteger(result->goal.y);
        lua->settable(-3);
        lua->pushstring("map");
        lua->pushinteger(result->map);
        lua->settable(-3);
        lua->pushstring("x");
        lua->pushinteger(result->at.x);
        lua->settable(-3);
        lua->pushstring("y");
        lua->pushinteger(result->at.y);
        lua->settable(-3);
        co_return 1;
    };
    return builder.run();
}

int builtin::brain::builtin_mobs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto& atlas = atlas::get();
    auto  mobs  = fb::model::table::mob;
    auto  index = lua_Integer{1};
    lua->new_table();
    for (auto& [oid, entry] : bot->sight.entries())
    {
        auto id = atlas.mob(bot->map(), entry.look, entry.color);
        if (id.has_value() == false)
            continue;

        auto model = mobs->find(id.value());
        lua->new_table();
        lua->pushstring("oid");
        lua->pushinteger(oid);
        lua->settable(-3);
        lua->pushstring("id");
        lua->pushinteger(id.value());
        lua->settable(-3);
        lua->pushstring("name");
        lua->pushstring(model != nullptr ? model->name : "");
        lua->settable(-3);
        lua->pushstring("x");
        lua->pushinteger(entry.position.x);
        lua->settable(-3);
        lua->pushstring("y");
        lua->pushinteger(entry.position.y);
        lua->settable(-3);
        if (model != nullptr)
        {
            lua->pushstring("hp");
            lua->pushinteger(model->hp);
            lua->settable(-3);
            lua->pushstring("damage");
            lua->pushinteger(model->damage.max);
            lua->settable(-3);
            lua->pushstring("attack");
            lua->pushstring(fb::model::enum_value::enum_tostring(model->attack_type));
            lua->settable(-3);
            lua->pushstring("speed");
            lua->pushinteger(std::chrono::duration_cast<std::chrono::milliseconds>(
                                 static_cast<std::chrono::microseconds>(model->speed))
                                 .count());
            lua->settable(-3);
        }
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

int builtin::brain::builtin_spawns(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto names = std::unordered_set<std::string>{};
    if (lua_istable(L, 3))
    {
        for (auto i = lua_Integer{1}; lua_rawgeti(L, 3, i) == LUA_TSTRING; i++)
        {
            names.insert(lua->tostring(-1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }

    auto mobs  = fb::model::table::mob;
    auto index = lua_Integer{1};
    lua->new_table();
    for (auto& spawn : atlas::get().spawns(bot->map()))
    {
        auto model = mobs->find(spawn.mob);
        if (model == nullptr || (names.empty() == false && names.contains(model->name) == false))
            continue;

        lua->new_table();
        lua->pushstring("name");
        lua->pushstring(model->name);
        lua->settable(-3);
        lua->pushstring("x");
        lua->pushinteger((spawn.begin.x + spawn.end.x) / 2);
        lua->settable(-3);
        lua->pushstring("y");
        lua->pushinteger((spawn.begin.y + spawn.end.y) / 2);
        lua->settable(-3);
        lua->pushstring("count");
        lua->pushinteger(spawn.count);
        lua->settable(-3);
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

int builtin::brain::builtin_attack(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto brain = lua->touserdata<fb::bot::brain>(1);
    if (brain == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(2);
    if (bot == nullptr)
        return 0;

    auto oid    = static_cast<uint32_t>(lua_tointeger(L, 3));
    auto tactic = fb::bot::brain::tactic{};
    if (lua_istable(L, 4))
    {
        lua_getfield(L, 4, "guard");
        tactic.guard = static_cast<uint32_t>(lua_tointeger(L, -1));
        lua_getfield(L, 4, "swing");
        tactic.swing = std::chrono::milliseconds(lua_tointeger(L, -1));
        lua_getfield(L, 4, "pace");
        tactic.pace = std::chrono::milliseconds(lua_tointeger(L, -1));
        lua_pop(L, 3);
    }
    auto result = std::make_shared<fb::bot::brain::event>();

    auto builder  = lua->new_co_builder();
    builder.yield = [brain, bot, oid, tactic, result]() -> async::task<void> {
        brain->act(bot->id, "attack");
        *result = co_await brain->attack(bot, oid, tactic);
    };
    builder.resume = [lua, result]() -> async::task<int> {
        lua->new_table();
        lua->pushstring("type");
        lua->pushstring(result->type);
        lua->settable(-3);
        lua->pushstring("map");
        lua->pushinteger(result->map);
        lua->settable(-3);
        lua->pushstring("x");
        lua->pushinteger(result->at.x);
        lua->settable(-3);
        lua->pushstring("y");
        lua->pushinteger(result->at.y);
        lua->settable(-3);
        lua->pushstring("count");
        lua->pushinteger(result->count);
        lua->settable(-3);
        lua->pushstring("gx");
        lua->pushinteger(result->goal.x);
        lua->settable(-3);
        lua->pushstring("gy");
        lua->pushinteger(result->goal.y);
        lua->settable(-3);
        co_return 1;
    };
    return builder.run();
}
