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

    auto model_ptr = std::make_shared<const fb::model::map*>();
    auto weak      = map->weak_from_this_as<fb::game::map>();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *model_ptr = &map->model;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushobject(**model_ptr);
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_width(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto width    = std::make_shared<uint16_t>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *width = map->width();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*width);
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_height(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto height   = std::make_shared<uint16_t>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *height = map->height();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*height);
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_area(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto width    = std::make_shared<uint16_t>();
    auto height   = std::make_shared<uint16_t>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *width  = map->width();
        *height = map->height();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*width);
        lua->pushinteger(*height);
        co_return 2;
    };
    return builder.run();
}

int builtin::map::builtin_objects(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto type     = lua->toenum(2, OBJECT_TYPE::OBJECT);
    auto objects  = std::make_shared<std::vector<std::shared_ptr<object>>>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (auto& [_, obj] : map->objects)
        {
            if (obj->is(type))
                objects->push_back(obj);
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (size_t i = 0; i < objects->size(); i++)
        {
            lua->pushobject((*objects)[i]);
            lua_rawseti(L, -2, static_cast<int>(i + 1));
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    if (!lua->is_table(2))
        return 0;

    lua->rawgeti(2, 1);
    auto x = (uint16_t)lua->tointeger(-1);
    lua->remove(-1);
    lua->rawgeti(2, 2);
    auto y = (uint16_t)lua->tointeger(-1);
    lua->remove(-1);

    auto type     = lua->toenum(3, OBJECT_TYPE::OBJECT);
    auto nears    = std::make_shared<std::vector<std::shared_ptr<object>>>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *nears = map->nears(fb::model::point16_t{x, y}, type);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (size_t i = 0; i < nears->size(); i++)
        {
            lua->pushobject((*nears)[i]);
            lua_rawseti(L, -2, static_cast<int>(i + 1));
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_movable(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
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
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        auto  role   = ROLE::USER;
        if (obj->is(OBJECT_TYPE::CHARACTER))
            role = static_cast<character*>(obj.get())->role();

        auto pos = position;
        if (is_front)
            pos = obj->front_position(step);

        co_await server.threads.switching(map_weak);

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

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto ch = lua->touserdata<character>(2);
    if (ch == nullptr)
        return 0;

    auto door_ptr = std::make_shared<fb::game::door*>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *door_ptr = map->doors.find(*ch);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*door_ptr == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*door_ptr);
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_doors(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto doors    = std::make_shared<std::vector<fb::game::door*>>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (const auto& entry : map->doors)
            doors->push_back(&entry.second);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (size_t i = 0; i < doors->size(); i++)
        {
            lua->pushobject(*(*doors)[i]);
            lua_rawseti(L, -2, static_cast<int>(i + 1));
        }
        co_return 1;
    };
    return builder.run();
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

    auto result   = std::make_shared<bool>(false);
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (auto& [fd, obj] : map->objects)
        {
            if (obj == you)
            {
                *result = true;
                break;
            }
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_belows(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x        = lua->tointeger(2);
    auto y        = lua->tointeger(3);
    auto type     = lua->toenum(4, OBJECT_TYPE::OBJECT);
    auto belows   = std::make_shared<std::vector<std::shared_ptr<object>>>();
    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (const auto& below : map->belows(fb::model::point16_t(x, y), type))
            belows->push_back(below);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (size_t i = 0; i < belows->size(); i++)
        {
            lua->pushinteger(static_cast<int>(i + 1));
            lua->pushobject((*belows)[i]);
            lua_settable(L, -3);
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::map::builtin_tile(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto map  = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x = (uint16_t)lua->tointeger(2);
    auto y = (uint16_t)lua->tointeger(3);

    if (argc >= 4)
    {
        auto value    = lua->tointeger(4);
        auto weak     = map->weak_from_this_as<fb::game::map>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            auto& server = static_cast<fb::game::server&>(lua->executor);
            auto  tile   = (*map)(x, y);
            if (tile != nullptr)
            {
                tile->object    = value;
                const auto area = fb::model::area<uint16_t>(x, y, x + 1, y + 1);
                server.maps.update_map_cache(map->model.id, area);
            }
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else
    {
        struct tile_snapshot
        {
            bool     found   = false;
            uint16_t id      = 0;
            uint16_t object  = 0;
            bool     blocked = false;
        };

        auto snapshot = std::make_shared<tile_snapshot>();
        auto weak     = map->weak_from_this_as<fb::game::map>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            auto tile = (*map)(x, y);
            if (tile != nullptr)
            {
                snapshot->found   = true;
                snapshot->id      = tile->id;
                snapshot->object  = tile->object;
                snapshot->blocked = tile->blocked;
            }
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (snapshot->found == false)
                co_return 0;
            lua->pushinteger(snapshot->id);
            lua->pushinteger(snapshot->object);
            lua->pushboolean(snapshot->blocked);
            co_return 3;
        };
        return builder.run();
    }
}

int builtin::map::builtin_at(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto map  = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x        = (uint16_t)lua->tointeger(2);
    auto y        = (uint16_t)lua->tointeger(3);
    auto type     = lua->toenum(4, OBJECT_TYPE::OBJECT);
    auto position = fb::model::point16_t{x, y};

    auto weak          = map->weak_from_this_as<fb::game::map>();
    auto result_holder = std::make_shared<std::shared_ptr<object>>(nullptr);
    auto builder       = lua->new_co_builder();
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

    auto argc = lua->argc();
    auto map  = lua->touserdata<fb::game::map>(1);
    if (map == nullptr)
        return 0;

    auto x         = (uint16_t)lua->tointeger(2);
    auto y         = (uint16_t)lua->tointeger(3);
    auto set_block = argc >= 4;
    auto block_opt = set_block ? lua->toboolean(4) : false;
    auto result    = std::make_shared<bool>();
    auto weak      = map->weak_from_this_as<fb::game::map>();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        if (set_block)
            *result = map->block(x, y, block_opt);
        else
            *result = map->blocked(x, y);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
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

    auto weak     = map->weak_from_this_as<fb::game::map>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        map->bulk_update(oids);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}