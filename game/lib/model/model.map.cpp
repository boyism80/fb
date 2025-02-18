#include <fb/model/model.h>
#include <fb/game/context.h>

int fb::model::map::builtin_id(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    thread->pushinteger(map->id);
    return 1;
}

int fb::model::map::builtin_name(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    thread->pushstring(map->name);
    return 1;
}

int fb::model::map::builtin_root(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto map = thread->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    if (ctx->model.map.contains(map->root) == false)
    {
        thread->pushnil();
        return 1;
    }

    auto& root = ctx->model.map[map->root];
    thread->pushobject(root);
    return 1;
}

int fb::model::map::builtin_cardinal(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx  = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    if (argc != 2)
    {
        thread->pushnil();
        return 1;
    }

    auto map = thread->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    auto direction = static_cast<CARDINAL_DIRECTION>(thread->tointeger(2));
    if (map->cardinal.contains(direction) == false)
    {
        thread->pushnil();
        return 1;
    }

    auto& dsl = map->cardinal.at(direction);
    if (dsl.header != DSL::map)
    {
        thread->pushnil();
        return 1;
    }

    auto params = dsl::map(dsl.params);
    if (ctx->model.map.contains(params.id) == false)
    {
        thread->pushnil();
        return 1;
    }

    thread->new_table();
    {
        thread->pushinteger(1);
        thread->pushobject(ctx->model.map[params.id]);
        lua_settable(*thread, -3);

        thread->pushinteger(2);
        thread->pushinteger(params.x);
        lua_settable(*thread, -3);

        thread->pushinteger(3);
        thread->pushinteger(params.y);
        lua_settable(*thread, -3);

        thread->pushinteger(4);
        thread->pushinteger(params.right);
        lua_settable(*thread, -3);

        thread->pushinteger(5);
        thread->pushinteger(params.bottom);
        lua_settable(*thread, -3);
    }
    return 1;
}

int fb::model::map::builtin_revive(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto map = thread->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    if (map->revive.size() == 0)
    {
        thread->pushnil();
        return 1;
    }

    thread->new_table();
    for (auto& [direction, x] : map->revive)
    {
        thread->pushinteger(direction);
        thread->pushobject(ctx->model.map[x]);
        lua_settable(*thread, -3);
    }

    return 1;
}

int fb::model::map::builtin_option(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ctx = thread->env<fb::game::context>("context");
    auto map = thread->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    thread->pushinteger(map->option);
    return 1;
}

fb::model::map* fb::model::__map::name2map(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }

    return nullptr;
}