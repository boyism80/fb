#include <fb/game/mob.h>
#include <fb/game/server.h>
#include <fb/game/builtin/object.h>
#include <fb/game/builtin/mob.h>
#include <fb/model/model.h>
#include <fb/lua.h>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(mob, "fb.game.mob")
{"__eq",                builtin::object::builtin_eq},
{"target",              builtin::mob::builtin_target},
{"oblivion",            builtin::mob::builtin_oblivion},
{"owner",               builtin::mob::builtin_owner},
{"items",               builtin::mob::builtin_items},
END_LUA_EXTENSION; // clang-format on

int builtin::mob::builtin_target(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto mob    = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = mob->weak_from_this_as<fb::game::mob>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto target = mob->target();
            return lua->ensure_resume(*server, weak, [=]() {
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
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            mob->target(target);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::mob::builtin_oblivion(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto mob    = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = mob->weak_from_this_as<fb::game::mob>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto oblivion = mob->oblivion();
            return lua->ensure_resume(*server, weak, [=]() {
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
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            mob->oblivion(oblivion);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::mob::builtin_owner(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto mob    = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto weak = mob->weak_from_this_as<fb::game::mob>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        return lua->ensure_resume(*server, weak, [=]() {
            auto owner = mob->owner.lock();
            if (owner == nullptr)
                lua->pushnil();
            else
                lua->pushobject(owner);
            return 1;
        });
    });
}

int builtin::mob::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto mob    = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto weak = mob->weak_from_this_as<fb::game::mob>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto buffer = mob->items();
        return lua->ensure_resume(*server, weak, [=]() {
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
