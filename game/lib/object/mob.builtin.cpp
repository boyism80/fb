#include <fb/game/mob.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::mob, "fb.game.mob")
{"__eq",                fb::game::object::builtin_eq},
{"target",              fb::game::mob::builtin_target},
{"oblivion",            fb::game::mob::builtin_oblivion},
{"owner",               fb::game::mob::builtin_owner},
{"items",               fb::game::mob::builtin_items},
END_LUA_EXTENSION; // clang-format on

int mob::builtin_target(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr || ctx->alive(*mob) == false)
        return 0;

    auto target = lua->touserdata<fb::game::life>(2);
    auto n      = (argc == 1 ? 1 : 0);
    return ctx->builtin(*mob, lua, n, [=]() {
        if (argc == 1)
        {
            if (mob->_target == nullptr)
                lua->pushnil();
            else
                lua->pushobject(mob->_target);
        }
        else
        {
            mob->target(target);
        }
    });
}

int mob::builtin_oblivion(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr || ctx->alive(*mob) == false)
        return 0;

    auto oblivion = lua->touserdata<fb::game::life>(2);
    auto n        = (argc == 1 ? 1 : 0);
    return ctx->builtin(*mob, lua, n, [=]() {
        if (argc == 1)
        {
            if (mob->_oblivion == nullptr)
                lua->pushnil();
            else
                lua->pushobject(mob->_oblivion);
        }
        else
        {
            mob->oblivion(oblivion);
        }
    });
}

int mob::builtin_owner(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr || ctx->alive(*mob) == false)
        return 0;

    return ctx->builtin(*mob, lua, 1, [=]() {
        if (ctx->alive(*mob->owner))
            lua->pushobject(mob->owner);
        else
            lua->pushnil();
    });
}

int mob::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr || ctx->alive(*mob) == false)
        return 0;

    return ctx->builtin(*mob, lua, 1, [=]() {
        lua->new_table();
        auto i = 0;
        for (auto item : mob->items())
        {
            lua->pushobject(item);
            lua_rawseti(L, -2, i + 1);
            i++;
        }
    });
}