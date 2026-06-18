#include <fb/game/server.h>
#include <fb/game/appearance.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

#include <stdexcept>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::object, "fb.model.object")
{"name",                builtin::model::object::builtin_name},
{"look",                builtin::model::object::builtin_look},
{"color",               builtin::model::object::builtin_color},
{"appearance",          builtin::model::object::builtin_appearance},
END_LUA_EXTENSION; // clang-format on

int builtin::model::object::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto object = lua->touserdata<fb::model::object>(1);
    if (object == nullptr)
        return 0;

    lua->pushstring(object->name);
    return 1;
}

int builtin::model::object::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto object = lua->touserdata<fb::model::object>(1);
    if (object == nullptr)
        return 0;

    lua->pushinteger(object->look);
    return 1;
}

int builtin::model::object::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto object = lua->touserdata<fb::model::object>(1);
    if (object == nullptr)
        return 0;

    lua->pushinteger(object->color);
    return 1;
}

int builtin::model::object::builtin_appearance(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto obj = lua->touserdata<fb::model::object>(1);
    if (obj == nullptr)
        return 0;

    try
    {
        obj->create_appearance()->to_lua(lua);
        return 1;
    }
    catch (const std::exception&)
    {
        lua->pushnil();
        return 1;
    }
}