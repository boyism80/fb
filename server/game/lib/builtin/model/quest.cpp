#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::quest, "fb.model.quest")
{"step",                builtin::model::quest::builtin_step},
{"progress",            builtin::model::quest::builtin_progress},
{"reward",              builtin::model::quest::builtin_reward},
END_LUA_EXTENSION; // clang-format on

int builtin::model::quest::builtin_step(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto model  = lua->touserdata<fb::model::quest>(1);
    if (model == nullptr)
        return 0;

    lua->pushinteger(server->model.quest[model->parent].size());
    return 1;
}

int builtin::model::quest::builtin_progress(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::quest>(1);
    if (model == nullptr)
        return 0;

    lua->pushinteger(model->progress);
    return 1;
}

int builtin::model::quest::builtin_reward(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto model  = lua->touserdata<fb::model::quest>(1);
    if (model == nullptr)
        return 0;

    lua->pushstring(server->model.quest_attribute[model->parent].reward);
    return 1;
}