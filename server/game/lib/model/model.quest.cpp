#include <fb/model/model.h>
#include <fb/game/server.h>

int fb::model::quest::builtin_step(lua_State* L)
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

int fb::model::quest::builtin_progress(lua_State* L)
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

int fb::model::quest::builtin_reward(lua_State* L)
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