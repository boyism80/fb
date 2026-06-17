#include <fb/game/builtin/quest.h>
#include <fb/game/server.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(quest, "fb.game.quest")
{"step",                builtin::quest::builtin_step},
{"progress",            builtin::quest::builtin_progress},
{"param",               builtin::quest::builtin_param},
{"inc_progress",        builtin::quest::builtin_inc_progress},
{"inc_step",            builtin::quest::builtin_inc_step},
{"complete",            builtin::quest::builtin_complete},
{"completed",           builtin::quest::builtin_completed},
{"resume",              builtin::quest::builtin_resume},
END_LUA_EXTENSION; // clang-format on

int builtin::quest::builtin_step(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc >= 2)
    {
        auto new_step = lua->tointeger(2);
        auto weak     = owner->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            quest->step(new_step);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else
    {
        auto step_value = std::make_shared<uint32_t>();
        auto weak       = owner->weak_from_this_as<fb::game::character>();
        auto builder    = lua->new_co_builder(*server);
        builder.weak    = weak;
        builder.yield   = [=]() -> async::task<void> {
            *step_value = quest->step();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*step_value);
            co_return 1;
        };
        return builder.run();
    }
}

int builtin::quest::builtin_progress(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc >= 2)
    {
        auto new_progress = lua->tointeger(2);
        auto weak         = owner->weak_from_this_as<fb::game::character>();
        auto builder      = lua->new_co_builder(*server);
        builder.weak      = weak;
        builder.yield     = [=]() -> async::task<void> {
            quest->progress(new_progress);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else
    {
        auto progress_value = std::make_shared<uint32_t>();
        auto weak           = owner->weak_from_this_as<fb::game::character>();
        auto builder        = lua->new_co_builder(*server);
        builder.weak        = weak;
        builder.yield       = [=]() -> async::task<void> {
            *progress_value = quest->progress();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*progress_value);
            co_return 1;
        };
        return builder.run();
    }
}

int builtin::quest::builtin_param(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    if (argc >= 2)
    {
        auto param    = lua->tostring(2);
        auto weak     = owner->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            quest->param(param);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else
    {
        auto param_value = std::make_shared<std::string>();
        auto weak        = owner->weak_from_this_as<fb::game::character>();
        auto builder     = lua->new_co_builder(*server);
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            *param_value = quest->param();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushstring(param_value->c_str());
            co_return 1;
        };
        return builder.run();
    }
}

int builtin::quest::builtin_inc_progress(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    auto value = lua->tointeger(2, 1);
    if (value <= 0)
        return 0;

    auto weak     = owner->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *success = quest->inc_progress(value);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
}

int builtin::quest::builtin_inc_step(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    auto value = lua->tointeger(2, 1);
    if (value <= 0)
        return 0;

    auto weak     = owner->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *success = quest->inc_step(value);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
}

int builtin::quest::builtin_complete(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    auto weak     = owner->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *success = quest->complete();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
}

int builtin::quest::builtin_completed(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    auto weak     = owner->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *success = quest->completed();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
}

int builtin::quest::builtin_resume(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto quest  = lua->touserdata<fb::game::quest>(1);
    if (quest == nullptr)
        return 0;

    auto owner = quest->owner.lock();
    if (owner == nullptr)
        return 0;

    auto weak     = owner->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *success = quest->resume();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
}
