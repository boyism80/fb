#include <map.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::map, "fb.game.map")
{"width",               fb::game::map::builtin_width},
{"height",              fb::game::map::builtin_height},
{"area",                fb::game::map::builtin_area},
{"objects",             fb::game::map::builtin_objects},
{"movable",             fb::game::map::builtin_movable},
{"door",                fb::game::map::builtin_door},
{"doors",               fb::game::map::builtin_doors},
{"contains",            fb::game::map::builtin_contains},
END_LUA_EXTENSION; // clang-format on

int fb::game::map::builtin_width(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    thread->pushinteger(map->width());
    return 1;
}

int fb::game::map::builtin_height(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    thread->pushinteger(map->height());
    return 1;
}

int fb::game::map::builtin_area(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    thread->pushinteger(map->width());
    thread->pushinteger(map->height());
    return 2;
}

int fb::game::map::builtin_objects(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    thread->new_table();
    const auto& objects = map->objects;

    for (int i = 0; i < objects.size(); i++)
    {
        thread->pushobject(map->objects[i]);
        lua_rawseti(lua, -2, i + 1);
    }

    return 1;
}

int fb::game::map::builtin_movable(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto position = point16_t();

    if (lua_istable(lua, 2))
    {
        lua_rawgeti(lua, 2, 1);
        position.x = (uint16_t)thread->tointeger(-1);
        lua_remove(lua, -1);

        lua_rawgeti(lua, 2, 2);
        position.y = (uint16_t)thread->tointeger(-1);
        lua_remove(lua, -1);
    }
    else if (lua_isnumber(lua, 2) && lua_isnumber(lua, 3))
    {
        position.x = (uint16_t)thread->tointeger(2);
        position.y = (uint16_t)thread->tointeger(3);
    }
    else
    {
        thread->pushboolean(false);
        return 1;
    }

    thread->pushboolean(map->movable(position));
    return 1;
}

int fb::game::map::builtin_door(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto session = thread->touserdata<character>(2);
    if (session == nullptr)
        return 0;

    auto door = map->doors.find(*session);
    if (door == nullptr)
        thread->pushnil();
    else
        thread->pushobject(door);

    return 1;
}

int fb::game::map::builtin_doors(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    thread->new_table();

    auto i = 0;
    for (const auto& door : map->doors)
    {
        thread->pushobject(door.second);
        lua_rawseti(lua, -2, i + 1);
    }

    return 1;
}

int fb::game::map::builtin_contains(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto you = thread->touserdata<object>(1);
    if (you == nullptr)
        return 0;

    for (auto& [fd, obj] : map->objects)
    {
        if (obj == *you)
        {
            thread->pushboolean(true);
            return 1;
        }
    }

    thread->pushboolean(false);
    return 1;
}