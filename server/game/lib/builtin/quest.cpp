#include <fb/game/builtin/quest.h>
#include <fb/game/server.h>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(quest, "fb.game.quest")
{"model",               builtin::quest::builtin_model},
{"step",                builtin::quest::builtin_step},
{"progress",            builtin::quest::builtin_progress},
{"param",               builtin::quest::builtin_param},
{"inc_progress",        builtin::quest::builtin_inc_progress},
{"inc_step",            builtin::quest::builtin_inc_step},
{"complete",            builtin::quest::builtin_complete},
{"completed",           builtin::quest::builtin_completed},
{"completed_progress",  builtin::quest::builtin_completed_progress},
{"completed_step",      builtin::quest::builtin_completed_step},
END_LUA_EXTENSION; // clang-format on

int builtin::quest::builtin_model(lua_State* L)
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

    auto weak = owner->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto  step  = quest->step();
        auto& model = table::quest[quest->id][step];
        return lua->ensure_resume(*server, weak, [=, &model]() {
            lua->pushobject(model);
            return 1;
        });
    });
}

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
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            quest->step(new_step);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
    else
    {
        auto weak = owner->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto step_value = quest->step();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(step_value);
                return 1;
            });
        });
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
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            quest->progress(new_progress);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
    else
    {
        auto weak = owner->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto progress_value = quest->progress();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(progress_value);
                return 1;
            });
        });
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
        auto param = lua->tostring(2);
        auto weak  = owner->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            quest->param(param);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
    else
    {
        auto weak = owner->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto param_value = quest->param();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushstring(param_value);
                return 1;
            });
        });
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

    auto weak = owner->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto result = quest->inc_progress(value);
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
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

    auto weak = owner->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto result = quest->inc_step(value);
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
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

    auto weak = owner->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto result = quest->complete();
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
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

    auto weak = owner->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto result = quest->completed();
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
}

int builtin::quest::builtin_completed_progress(lua_State* L)
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

    auto weak = owner->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto& attr   = table::quest[quest->id];
        auto& model  = attr[quest->step()];
        auto  result = (model.progress == quest->progress());
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
}

int builtin::quest::builtin_completed_step(lua_State* L)
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

    auto weak = owner->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto result = (table::quest[quest->id].size() == quest->step());
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
}