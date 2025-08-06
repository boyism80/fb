#include <fb/game/quest.h>
#include <fb/game/server.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(quest, "fb.game.quest")
{"model",               quest::builtin_model},
{"step",                quest::builtin_step},
{"progress",            quest::builtin_progress},
{"param",               quest::builtin_param},
{"inc_progress",        quest::builtin_inc_progress},
{"inc_step",            quest::builtin_inc_step},
{"complete",            quest::builtin_complete},
{"completed",           quest::builtin_completed},
{"completed_progress",  quest::builtin_completed_progress},
{"completed_step",      quest::builtin_completed_step},
END_LUA_EXTENSION; // clang-format on

int fb::game::quest::builtin_model(lua_State* L)
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
        auto& model = server->model.quest[quest->id][quest->_step];
        return lua->ensure_resume(*server, weak, [=, &model]() {
            lua->pushobject(model);
            return 1;
        });
    });
}

int fb::game::quest::builtin_step(lua_State* L)
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
            quest->_step = new_step;
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

int fb::game::quest::builtin_progress(lua_State* L)
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
            quest->_progress = new_progress;
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

int fb::game::quest::builtin_param(lua_State* L)
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

int fb::game::quest::builtin_inc_progress(lua_State* L)
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

int fb::game::quest::builtin_inc_step(lua_State* L)
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

int fb::game::quest::builtin_complete(lua_State* L)
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

int fb::game::quest::builtin_completed(lua_State* L)
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

int fb::game::quest::builtin_completed_progress(lua_State* L)
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
        auto& attr   = server->model.quest[quest->id];
        auto& model  = attr[quest->_step];
        auto  result = (model.progress == quest->_progress);
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
}

int fb::game::quest::builtin_completed_step(lua_State* L)
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
        auto result = (server->model.quest[quest->id].size() == quest->_step);
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
}