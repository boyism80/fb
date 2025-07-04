#include <fb/game/map.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(map, "fb.game.map")
{"model",               map::builtin::builtin_model},
{"width",               map::builtin::builtin_width},
{"height",              map::builtin::builtin_height},
{"area",                map::builtin::builtin_area},
{"objects",             map::builtin::builtin_objects},
{"nears",               map::builtin::builtin_nears},
{"movable",             map::builtin::builtin_movable},
{"door",                map::builtin::builtin_door},
{"doors",               map::builtin::builtin_doors},
{"contains",            map::builtin::builtin_contains},
{"belows",              map::builtin::builtin_belows},
{"tile",                map::builtin::builtin_tile},
{"at",                  map::builtin::builtin_at},
END_LUA_EXTENSION; // clang-format on

int map::builtin::builtin_model(lua_State* L)
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

int map::builtin::builtin_width(lua_State* L)
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

int map::builtin::builtin_height(lua_State* L)
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

int map::builtin::builtin_area(lua_State* L)
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

int map::builtin::builtin_objects(lua_State* L)
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

int map::builtin::builtin_nears(lua_State* L)
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

    auto type  = lua->toenum(3, OBJECT_TYPE::UNKNOWN);
    auto nears = map->nears(fb::model::point16_t{x, y}, type);

    lua->new_table();
    for (int i = 0; i < nears.size(); i++)
    {
        lua->pushobject(nears[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int map::builtin::builtin_movable(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto obj = lua->touserdata<fb::game::object>(2);
    if (obj == nullptr)
        return 0;

    auto position = fb::model::point16_t();
    if (lua_istable(L, 3))
    {
        lua_rawgeti(L, 3, 1);
        position.x = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);

        lua_rawgeti(L, 3, 2);
        position.y = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);
    }
    else if (lua_isnumber(L, 3) && lua_isnumber(L, 4))
    {
        position.x = (uint16_t)lua->tointeger(3);
        position.y = (uint16_t)lua->tointeger(4);
    }
    else if (lua_isuserdata(L, 3))
    {
        auto obj = lua->touserdata<object>(3);
        if (obj == nullptr)
            return 0;

        auto argc = lua->argc();
        auto step = lua->tointeger(4, 1);

        position = obj->front_position(step);
    }
    else
    {
        lua->pushboolean(false);
        return 1;
    }

    auto weak = obj->weak_from_this_as<object>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto result = map->movable(*obj, position);
        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushboolean(result);
            return 1;
        });
    });
}

int map::builtin::builtin_door(lua_State* L)
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

int map::builtin::builtin_doors(lua_State* L)
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

int map::builtin::builtin_contains(lua_State* L)
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

int map::builtin::builtin_belows(lua_State* L)
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
    auto type = lua->toenum(4, OBJECT_TYPE::UNKNOWN);

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

int map::builtin::builtin_tile(lua_State* L)
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
        for (auto& obj : map->nears(position, OBJECT_TYPE::CHARACTER))
        {
            auto ch = std::static_pointer_cast<character>(obj);
            ch->update_map(*map, position, fb::model::size8_t{1, 1});
        }
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

int map::builtin::builtin_at(lua_State* L)
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
    auto type     = lua->toenum(4, OBJECT_TYPE::UNKNOWN);
    auto position = fb::model::point16_t{x, y};

    auto weak = map->weak_from_this_as<fb::game::map>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto nears  = map->nears(fb::model::point16_t{x, y}, type);
        auto result = std::shared_ptr<object>(nullptr);
        for (auto obj : nears)
        {
            if (obj->position() == position)
            {
                result = obj;
                break;
            }
        }

        return lua->ensure_resume(*ctx, weak, [=]() {
            if (result == nullptr)
                lua->pushnil();
            else
                lua->pushobject(result);
            return 1;
        });
    });
}