#include <fb/game/builtin/map.h>
#include <fb/game/server.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(map, "fb.game.map")
{"model",               builtin::map::builtin_model},
{"width",               builtin::map::builtin_width},
{"height",              builtin::map::builtin_height},
{"area",                builtin::map::builtin_area},
{"objects",             builtin::map::builtin_objects},
{"nears",               builtin::map::builtin_nears},
{"movable",             builtin::map::builtin_movable},
{"door",                builtin::map::builtin_door},
{"doors",               builtin::map::builtin_doors},
{"contains",            builtin::map::builtin_contains},
{"belows",              builtin::map::builtin_belows},
{"tile",                builtin::map::builtin_tile},
{"at",                  builtin::map::builtin_at},
{"block",               builtin::map::builtin_block},
{"bulk_update",         builtin::map::builtin_bulk_update},
END_LUA_EXTENSION; // clang-format on

int builtin::map::builtin_model(lua_State* L)
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

int builtin::map::builtin_width(lua_State* L)
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

int builtin::map::builtin_height(lua_State* L)
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

int builtin::map::builtin_area(lua_State* L)
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

int builtin::map::builtin_objects(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto type = lua->toenum(2, OBJECT_TYPE::OBJECT);

    lua->new_table();
    int i = 0;
    for (auto& [_, obj] : map->objects)
    {
        if (obj->is(type) == false)
            continue;

        lua->pushobject(obj);
        lua_rawseti(L, -2, i + 1);
        i++;
    }

    return 1;
}

int builtin::map::builtin_nears(lua_State* L)
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

    auto type  = lua->toenum(3, OBJECT_TYPE::OBJECT);
    auto nears = map->nears(fb::model::point16_t{x, y}, type);

    lua->new_table();
    for (int i = 0; i < nears.size(); i++)
    {
        lua->pushobject(nears[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int builtin::map::builtin_movable(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto map    = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto obj = lua->touserdata<fb::game::object>(2);
    if (obj == nullptr)
        return 0;

    auto position = fb::model::point16_t();
    auto is_front = false;
    auto step     = 0;
    if (lua->is_table(3))
    {
        lua->rawgeti(3, 1);
        position.x = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);

        lua->rawgeti(3, 2);
        position.y = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);
    }
    else if (lua->is_number(3) && lua->is_number(4))
    {
        position.x = (uint16_t)lua->tointeger(3);
        position.y = (uint16_t)lua->tointeger(4);
    }
    else if (lua->is_number(3))
    {
        step     = lua->tointeger(3);
        is_front = true;
    }
    else
    {
        lua->pushboolean(false);
        return 1;
    }

    auto weak     = obj->weak_from_this_as<fb::game::object>();
    auto map_weak = map->weak_from_this_as<fb::game::map>();
    auto result   = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto role = ROLE::USER;
        if (obj->is(OBJECT_TYPE::CHARACTER))
            role = static_cast<character*>(obj.get())->role();

        auto pos = position;
        if (is_front)
            pos = obj->front_position(step);

        co_await server->threads.switching(map_weak);

        auto map_locked = map_weak.lock();
        if (map_locked == nullptr)
            co_return;

        *result = map_locked->movable(pos, [=](const auto& near_obj) -> bool {
            if (near_obj.is(OBJECT_TYPE::CHARACTER) == false)
                return true;

            auto& ch = static_cast<const character&>(near_obj);
            return !ch.hidden(role);
        });
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_door(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto map    = lua->touserdata<fb::game::map>(1);
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

int builtin::map::builtin_doors(lua_State* L)
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

int builtin::map::builtin_contains(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto you = lua->touserdata<object>(2);
    if (you == nullptr)
        return 0;

    for (auto& [fd, obj] : map->objects)
    {
        if (obj == you)
        {
            lua->pushboolean(true);
            return 1;
        }
    }

    lua->pushboolean(false);
    return 1;
}

int builtin::map::builtin_belows(lua_State* L)
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
    auto type = lua->toenum(4, OBJECT_TYPE::OBJECT);

    lua->new_table();
    auto i = 0;
    for (const auto& below : map->belows(fb::model::point16_t(x, y), type))
    {
        lua->pushinteger(i + 1);
        lua->pushobject(below);
        lua_settable(L, -3);

        i++;
    }
    return 1;
}

int builtin::map::builtin_tile(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto map    = lua->touserdata<fb::game::map>(1);
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

        const auto area = fb::model::area<uint16_t>(x, y, x + 1, y + 1);
        server->maps.update_map_cache(map->model.id, area);
        return 0;
    }
    else
    {
        lua->pushinteger(tile->id);
        lua->pushinteger(tile->object);
        lua->pushboolean(tile->blocked);
        return 3;
    }
}

int builtin::map::builtin_at(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto map    = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x        = (uint16_t)lua->tointeger(2);
    auto y        = (uint16_t)lua->tointeger(3);
    auto type     = lua->toenum(4, OBJECT_TYPE::OBJECT);
    auto position = fb::model::point16_t{x, y};

    auto weak          = map->weak_from_this_as<fb::game::map>();
    auto result_holder = std::make_shared<std::shared_ptr<object>>(nullptr);
    auto builder       = lua->new_co_builder(*server);
    builder.weak       = weak;
    builder.yield      = [=]() -> async::task<void> {
        auto nears = map->nears(fb::model::point16_t{x, y}, type);
        for (const auto& obj : nears)
        {
            if (obj->position() == position)
            {
                *result_holder = obj;
                break;
            }
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*result_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*result_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_block(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto map    = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x = (uint16_t)lua->tointeger(2);
    auto y = (uint16_t)lua->tointeger(3);

    if (argc >= 4)
    {
        auto option = lua->toboolean(4);
        if (map->block(x, y, option))
        {
            lua->pushboolean(true);
        }
        else
        {
            lua->pushboolean(false);
        }
        return 1;
    }

    lua->pushboolean(map->blocked(x, y));
    return 1;
}

int builtin::map::builtin_bulk_update(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    if (lua->is_table(2) == false)
        return 0;

    auto oids = std::vector<uint32_t>();
    auto size = lua->rawlen(2);
    for (auto i = 1; i <= size; i++)
    {
        for (int i = 0; i < size; i++)
        {
            lua->rawgeti(2, i + 1);
            if (lua->is_number(-1))
                oids.push_back((uint32_t)lua->tointeger(-1));
        }
    }

    map->bulk_update(oids);
    return 0;
}