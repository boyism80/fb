#include <fb/game/mob.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(mob, "fb.game.mob")
{"__eq",                object::builtin::builtin_eq},
{"target",              mob::builtin::builtin_target},
{"oblivion",            mob::builtin::builtin_oblivion},
{"owner",               mob::builtin::builtin_owner},
{"items",               mob::builtin::builtin_items},
END_LUA_EXTENSION; // clang-format on

int mob::builtin::builtin_target(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = mob->weak_from_this_as<fb::game::mob>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto target = mob->target();
            return lua->ensure_resume(*ctx, weak, [=]() {
                if (target == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(target);
                return 1;
            });
        });
    }
    else
    {
        auto target = lua->touserdata<fb::game::life>(2);
        auto weak   = mob->weak_from_this_as<fb::game::mob>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            mob->target(target);
            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int mob::builtin::builtin_oblivion(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = mob->weak_from_this_as<fb::game::mob>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto oblivion = mob->oblivion();
            return lua->ensure_resume(*ctx, weak, [=]() {
                if (oblivion == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(oblivion);
                return 1;
            });
        });
    }
    else
    {
        auto oblivion = lua->touserdata<fb::game::life>(2);
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        return lua->ensure_yield(*ctx, weak, [=]() {
            mob->oblivion(oblivion);
            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int mob::builtin::builtin_owner(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto weak = mob->weak_from_this_as<fb::game::mob>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        return lua->ensure_resume(*ctx, weak, [=]() {
            auto owner = mob->owner.lock();
            if (owner == nullptr)
                lua->pushnil();
            else
                lua->pushobject(owner);
            return 1;
        });
    });
}

int mob::builtin::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto weak = mob->weak_from_this_as<fb::game::mob>();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto buffer = mob->items();
        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->new_table();
            auto i = 0;
            for (auto item : buffer)
            {
                lua->pushobject(item);
                lua_rawseti(L, -2, i + 1);
                i++;
            }
            return 1;
        });
    });
}