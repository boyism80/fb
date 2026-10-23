#include <fb/game/builtin/state_machine.h>

#include <fb/game/state_machine.h>

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(state_machine, "fb.game.state_machine")
{"name",   builtin::state_machine::builtin_name},
{"get",    builtin::state_machine::builtin_get},
{"set",    builtin::state_machine::builtin_set},
{"every",  builtin::state_machine::builtin_every},
{"after",  builtin::state_machine::builtin_after},
{"cancel", builtin::state_machine::builtin_cancel},
{"call",   builtin::state_machine::builtin_call},
END_LUA_EXTENSION; // clang-format on

int builtin::state_machine::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto machine = lua->touserdata<fb::game::state_machine>(1);
    if (machine == nullptr)
        return 0;

    lua->pushstring(machine->name());
    return 1;
}

int builtin::state_machine::builtin_get(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto machine = lua->touserdata<fb::game::state_machine>(1);
    if (machine == nullptr)
        return 0;

    auto value = machine->property(std::string(lua->tostring(2)));
    if (value.has_value())
        value->push(*lua);
    else
        lua->pushnil();
    return 1;
}

int builtin::state_machine::builtin_set(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto machine = lua->touserdata<fb::game::state_machine>(1);
    if (machine == nullptr)
        return 0;

    auto key = std::string(lua->tostring(2));
    if (key.empty())
        throw std::runtime_error("state_machine:set requires a key");

    auto type = lua_type(L, 3);
    if (type == LUA_TNIL || type == LUA_TNONE)
    {
        machine->property(key, std::nullopt);
    }
    else if (type == LUA_TBOOLEAN || type == LUA_TNUMBER || type == LUA_TSTRING || type == LUA_TTABLE)
    {
        machine->property(key, fb::lua::value::copy_data(L, 3));
    }
    else
    {
        throw std::runtime_error("state_machine:set accepts nil, boolean, number, string or table");
    }
    return 0;
}

int builtin::state_machine::builtin_every(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto machine = lua->touserdata<fb::game::state_machine>(1);
    if (machine == nullptr)
        return 0;

    auto id          = std::string(lua->tostring(2));
    auto interval_ms = lua->tointeger(3);
    auto hook        = std::string(lua->tostring(4));
    if (id.empty() || hook.empty() || interval_ms <= 0)
        throw std::runtime_error("state_machine:every requires an id, a positive interval and a hook");

    machine->schedule(id, static_cast<uint32_t>(interval_ms), hook, true);
    return 0;
}

int builtin::state_machine::builtin_after(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto machine = lua->touserdata<fb::game::state_machine>(1);
    if (machine == nullptr)
        return 0;

    auto id       = std::string(lua->tostring(2));
    auto delay_ms = lua->tointeger(3);
    auto hook     = std::string(lua->tostring(4));
    if (id.empty() || hook.empty() || delay_ms <= 0)
        throw std::runtime_error("state_machine:after requires an id, a positive delay and a hook");

    machine->schedule(id, static_cast<uint32_t>(delay_ms), hook, false);
    return 0;
}

int builtin::state_machine::builtin_cancel(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto machine = lua->touserdata<fb::game::state_machine>(1);
    if (machine == nullptr)
        return 0;

    machine->cancel(std::string(lua->tostring(2)));
    return 0;
}

int builtin::state_machine::builtin_call(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto machine = lua->touserdata<fb::game::state_machine>(1);
    if (machine == nullptr)
        return 0;

    auto hook = std::string(lua->tostring(2));
    if (hook.empty())
        throw std::runtime_error("state_machine:call requires a hook");

    auto args = std::vector<fb::lua::value>{};
    for (auto i = 3; i <= lua->argc(); i++)
    {
        args.push_back(fb::lua::value::copy_data(L, i));
    }

    machine->call(std::move(hook), std::move(args));
    return 0;
}
