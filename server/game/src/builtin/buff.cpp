#include <fb/game/builtin/spell.h>
#include <fb/game/server.h>
#include <fb/game/spell.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(buff, "fb.game.buff")
{"model",              builtin::buff::builtin_model},
{"time",               builtin::buff::builtin_time},
END_LUA_EXTENSION; // clang-format on

int builtin::buff::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto buff = lua->touserdata<fb::game::buff>(1);
    if (buff == nullptr)
        return 0;

    lua->pushobject(buff->model());
    return 1;
}

int builtin::buff::builtin_time(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto buff = lua->touserdata<fb::game::buff>(1);
    if (buff == nullptr)
        return 0;

    // The buff timer reads and extends the duration on the owner's thread.
    auto weak         = buff->weak_from_this_as<fb::game::buff>();
    auto seconds      = argc == 1 ? 0 : lua->tointeger(2);
    auto remaining_ms = std::make_shared<int64_t>(0);
    auto builder      = lua->new_co_builder();
    builder.weak      = buff->owner;
    builder.yield     = [=]() -> async::task<void> {
        auto locked = weak.lock();
        if (locked == nullptr)
            co_return;

        if (argc == 1)
            *remaining_ms = (std::max)(static_cast<int64_t>(locked->remaining().total_milliseconds()), int64_t(0));
        else
            locked->remaining(fb::model::timespan{std::chrono::seconds(seconds)});
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (argc == 1)
        {
            lua->pushinteger(static_cast<lua_Integer>(*remaining_ms / 1000));
            co_return 1;
        }
        else
        {
            co_return 0;
        }
    };
    return builder.run();
}