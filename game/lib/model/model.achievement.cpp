#include <fb/model/model.h>

int fb::model::achievement::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context     = lua->env<fb::game::context>("context");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushinteger(achievement->id);
    return 1;
}

int fb::model::achievement::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context     = lua->env<fb::game::context>("context");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushinteger(achievement->look);
    return 1;
}

int fb::model::achievement::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context     = lua->env<fb::game::context>("context");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushinteger(achievement->color);
    return 1;
}

int fb::model::achievement::builtin_text(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context     = lua->env<fb::game::context>("context");
    auto achievement = lua->touserdata<fb::model::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushstring(achievement->text);
    return 1;
}