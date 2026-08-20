#include <fb/game/castle.h>
#include <fb/game/server.h>
#include <fb/game/builtin/castle.h>
#include <macro.h>

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

    auto& owner = castle->owner_clan_id();
    if (owner.has_value())
        lua->pushinteger(owner.value());
    else
        lua->pushnil();
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

    lua->pushboolean(castle->siege_active());
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

    auto remaining = castle->siege_remaining_seconds();
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
    castle->start_siege(duration_seconds);
    return 0;
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
    auto error        = std::make_shared<std::optional<std::string>>();
    auto builder      = lua->new_co_builder();
    builder.yield     = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            co_await server.castles.end_siege(divine_beast, winner_clan_id);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}
