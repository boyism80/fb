#include <fb/game/builtin/castle.h>

#include <fb/game/castle.h>
#include <fb/game/server.h>
#include <macro.h>

#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(castle, "fb.game.castle")
{"divine_beast",        builtin::castle::builtin_divine_beast},
{"owner_clan",          builtin::castle::builtin_owner_clan},
{"siege_active",        builtin::castle::builtin_siege_active},
{"siege_end_time",      builtin::castle::builtin_siege_end_time},
{"start_siege",         builtin::castle::builtin_start_siege},
{"end_siege",           builtin::castle::builtin_end_siege},
END_LUA_EXTENSION; // clang-format on

int builtin::castle::builtin_divine_beast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto castle = lua->touserdata<fb::game::castle>(1);
    if (castle == nullptr)
        return 0;

    lua->pushinteger(static_cast<uint32_t>(castle->divine_beast()));
    return 1;
}

int builtin::castle::builtin_owner_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto castle = lua->touserdata<fb::game::castle>(1);
    if (castle == nullptr)
        return 0;

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  guard = srv.castles.try_enter_read(castle->id());
    if (guard.has_value() == false || guard->value() == nullptr || guard->value()->owner_clan_id().has_value() == false)
        lua->pushnil();
    else
        lua->pushinteger(guard->value()->owner_clan_id().value());
    return 1;
}

int builtin::castle::builtin_siege_active(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto castle = lua->touserdata<fb::game::castle>(1);
    if (castle == nullptr)
        return 0;

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  guard = srv.castles.try_enter_read(castle->id());
    lua->pushboolean(guard.has_value() && guard->value() != nullptr && guard->value()->siege_active());
    return 1;
}

int builtin::castle::builtin_siege_end_time(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto castle = lua->touserdata<fb::game::castle>(1);
    if (castle == nullptr)
        return 0;

    auto& srv       = static_cast<fb::game::server&>(lua->executor);
    auto  guard     = srv.castles.try_enter_read(castle->id());
    auto  remaining = std::optional<uint32_t>{};
    if (guard.has_value() && guard->value() != nullptr)
        remaining = guard->value()->siege_remaining_seconds();

    if (remaining.has_value())
        lua->pushinteger(remaining.value());
    else
        lua->pushnil();
    return 1;
}

int builtin::castle::builtin_start_siege(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto castle = lua->touserdata<fb::game::castle>(1);
    if (castle == nullptr)
        return 0;

    auto duration_seconds = static_cast<uint32_t>(lua->tointeger(2));
    auto divine_beast     = castle->divine_beast();
    auto builder          = lua->new_co_builder();
    builder.yield         = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        co_await server.castles.start_siege(divine_beast, duration_seconds);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::castle::builtin_end_siege(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto castle = lua->touserdata<fb::game::castle>(1);
    if (castle == nullptr)
        return 0;

    auto winner_clan_id = std::optional<uint32_t>{};
    if (lua->is_number(2))
        winner_clan_id = static_cast<uint32_t>(lua->tointeger(2));

    auto divine_beast = castle->divine_beast();
    auto result       = std::make_shared<std::pair<fb::game::castle::siege_end_result, std::optional<uint32_t>>>();
    auto builder      = lua->new_co_builder();
    builder.yield     = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        *result      = co_await server.castles.end_siege(divine_beast, winner_clan_id);
    };
    builder.resume = [=]() -> async::task<int> {
        switch (result->first)
        {
        case fb::game::castle::siege_end_result::ENDED:
            lua->pushstring("ended");
            break;

        case fb::game::castle::siege_end_result::SAVE_FAILED:
            lua->pushstring("save_failed");
            break;

        default:
            lua->pushstring("not_active");
            break;
        }

        if (result->second.has_value())
            lua->pushinteger(result->second.value());
        else
            lua->pushnil();
        co_return 2;
    };
    return builder.run();
}
