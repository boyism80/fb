#include <fb/game/map.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::map, "fb.game.map")
{"model",               fb::game::map::builtin_model},
{"width",               fb::game::map::builtin_width},
{"height",              fb::game::map::builtin_height},
{"area",                fb::game::map::builtin_area},
{"objects",             fb::game::map::builtin_objects},
{"nears",               fb::game::map::builtin_nears},
{"movable",             fb::game::map::builtin_movable},
{"door",                fb::game::map::builtin_door},
{"doors",               fb::game::map::builtin_doors},
{"contains",            fb::game::map::builtin_contains},
{"belows",              fb::game::map::builtin_belows},
{"tile",                fb::game::map::builtin_tile},
{"at",                  fb::game::map::builtin_at},
END_LUA_EXTENSION; // clang-format on

int fb::game::map::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushobject(map->model);
    return 1;
}

int fb::game::map::builtin_width(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushinteger(map->width());
    return 1;
}

int fb::game::map::builtin_height(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushinteger(map->height());
    return 1;
}

int fb::game::map::builtin_area(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    lua->pushinteger(map->width());
    lua->pushinteger(map->height());
    return 2;
}

int fb::game::map::builtin_objects(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    lua->new_table();
    const auto& objects = map->objects;

    int i = 0;
    for (auto& [_, obj] : map->objects)
    {
        lua->pushobject(obj);
        lua_rawseti(L, -2, i + 1);
        i++;
    }

    return 1;
}

int fb::game::map::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto map  = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    uint16_t x, y;
    if (!lua->is_table(2))
        return 0;

    lua->rawgeti(2, 1);
    x = (uint16_t)lua->tointeger(-1);
    lua->remove(-1);
    lua->rawgeti(2, 2);
    y = (uint16_t)lua->tointeger(-1);
    lua->remove(-1);

    auto type  = argc < 3 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(lua->tointeger(3));
    auto nears = map->nears(fb::model::point16_t{x, y}, type);

    lua->new_table();
    for (int i = 0; i < nears.size(); i++)
    {
        lua->pushobject(nears[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int fb::game::map::builtin_movable(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto position = fb::model::point16_t();
    if (lua_istable(L, 2))
    {
        lua_rawgeti(L, 2, 1);
        position.x = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);

        lua_rawgeti(L, 2, 2);
        position.y = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);
    }
    else if (lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        position.x = (uint16_t)lua->tointeger(2);
        position.y = (uint16_t)lua->tointeger(3);
    }
    else if (lua_isuserdata(L, 2))
    {
        auto obj = lua->touserdata<object>(2);
        if (obj == nullptr)
            return 0;

        auto argc = lua->argc();
        auto step = argc < 3 ? 1 : lua->tointeger(3);

        position = obj->front_position(step);
    }
    else
    {
        lua->pushboolean(false);
        return 1;
    }

    return ctx->builtin(*map, lua, 1, [=]() -> async::task<void> {
        lua->pushboolean(map->movable(position));
        co_return;
    });
}

int fb::game::map::builtin_door(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto ch = lua->touserdata<character>(2);
    if (ch == nullptr)
        return 0;

    auto door = map->doors.find(*ch);
    if (door == nullptr)
        lua->pushnil();
    else
        lua->pushobject(door);

    return 1;
}

int fb::game::map::builtin_doors(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    lua->new_table();

    auto i = 0;
    for (const auto& door : map->doors)
    {
        lua->pushobject(door.second);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int fb::game::map::builtin_contains(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto you = lua->touserdata<object>(1);
    if (you == nullptr)
        return 0;

    for (auto& [fd, obj] : map->objects)
    {
        if (obj == *you)
        {
            lua->pushboolean(true);
            return 1;
        }
    }

    lua->pushboolean(false);
    return 1;
}

int fb::game::map::builtin_belows(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto map  = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x    = lua->tointeger(2);
    auto y    = lua->tointeger(3);
    auto type = argc < 4 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(lua->tointeger(4));

    lua->new_table();
    auto i = 0;
    for (auto below : map->belows(fb::model::point16_t(x, y), type))
    {
        lua->pushinteger(i + 1);
        lua->pushobject(below);
        lua_settable(L, -3);

        i++;
    }
    return 1;
}

int fb::game::map::builtin_tile(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto map  = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x    = (uint16_t)lua->tointeger(2);
    auto y    = (uint16_t)lua->tointeger(3);
    auto tile = (*map)(x, y);
    if (tile == nullptr)
        return 0;

    if (argc >= 4)
    {
        auto value   = lua->tointeger(4);
        tile->object = value;

        auto position = fb::model::point16_t{x, y};
        for (auto obj : map->nears(position, OBJECT_TYPE::CHARACTER))
        {
            auto ch = static_cast<character*>(obj);
            ch->update_map(*map, position, fb::model::size8_t{1, 1});
        }
    }
    else
    {
        lua->pushinteger(tile->id);
        lua->pushinteger(tile->object);
        lua->pushboolean(tile->blocked);
        return 3;
    }
}

int fb::game::map::builtin_at(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto map  = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x        = (uint16_t)lua->tointeger(2);
    auto y        = (uint16_t)lua->tointeger(3);
    auto position = fb::model::point16_t{x, y};
    auto type     = argc < 4 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(lua->tointeger(4));

    return ctx->builtin(*map, lua, 1, [=]() -> async::task<void> {
        auto nears = map->nears(fb::model::point16_t{x, y}, type);
        for (auto obj : nears)
        {
            if (obj->position() == position)
            {
                lua->pushobject(obj);
                co_return;
            }
        }

        lua->pushnil();
        co_return;
    });
}