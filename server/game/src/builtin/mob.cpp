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
{"parts",               builtin::mob::builtin_parts},
{"body",                builtin::mob::builtin_body},
{"parts_mode",          builtin::mob::builtin_parts_mode},
{"hide",                builtin::mob::builtin_hide},
END_LUA_EXTENSION; // clang-format on

int builtin::mob::builtin_target(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto target_holder = std::make_shared<std::shared_ptr<fb::game::life>>();
        auto weak          = mob->weak_from_this_as<fb::game::mob>();
        auto builder       = lua->new_co_builder();
        builder.weak       = weak;
        builder.yield      = [=]() -> async::task<void> {
            *target_holder = mob->target();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*target_holder == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*target_holder);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto target   = lua->touserdata<fb::game::life>(2);
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            mob->target(target);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::mob::builtin_oblivion(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto oblivion_holder = std::make_shared<std::shared_ptr<fb::game::life>>();
        auto weak            = mob->weak_from_this_as<fb::game::mob>();
        auto builder         = lua->new_co_builder();
        builder.weak         = weak;
        builder.yield        = [=]() -> async::task<void> {
            *oblivion_holder = mob->oblivion();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*oblivion_holder == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*oblivion_holder);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto oblivion = lua->touserdata<fb::game::life>(2);
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            mob->oblivion(oblivion);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::mob::builtin_owner(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto owner_holder = std::make_shared<std::shared_ptr<fb::game::character>>();
    auto weak         = mob->weak_from_this_as<fb::game::mob>();
    auto builder      = lua->new_co_builder();
    builder.weak      = weak;
    builder.yield     = [=]() -> async::task<void> {
        *owner_holder = mob->owner.lock();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*owner_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*owner_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::mob::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto buffer   = std::make_shared<fb::game::mob::item_vector_t>();
    auto weak     = mob->weak_from_this_as<fb::game::mob>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *buffer = mob->items();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        auto i = 0;
        for (const auto& item : *buffer)
        {
            lua->pushobject(item);
            lua_rawseti(L, -2, i + 1);
            i++;
        }
        co_return 1;
    };
    return builder.run();
}

int builtin::mob::builtin_parts(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buffer   = std::make_shared<fb::game::mob::parts_vector_t>();
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *buffer = mob->parts();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->new_table();
            for (size_t i = 0; i < buffer->size(); i++)
            {
                lua->pushobject((*buffer)[i]);
                lua_rawseti(L, -2, static_cast<int>(i + 1));
            }
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto part     = lua->touserdata<fb::game::mob>(2);
        auto result   = std::make_shared<bool>(false);
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *result = mob->add_part(part);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*result);
            co_return 1;
        };
        return builder.run();
    }
}

int builtin::mob::builtin_body(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    auto body_holder = std::make_shared<std::shared_ptr<fb::game::mob>>();
    auto weak        = mob->weak_from_this_as<fb::game::mob>();
    auto builder     = lua->new_co_builder();
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        *body_holder = mob->body();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*body_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*body_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::mob::builtin_parts_mode(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto mode     = std::make_shared<MOB_PARTS_MODE>(MOB_PARTS_MODE::PARTS);
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *mode = mob->parts_mode();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(static_cast<lua_Integer>(*mode));
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto mode     = static_cast<MOB_PARTS_MODE>(lua->tointeger(2));
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            mob->parts_mode(mode);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::mob::builtin_hide(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto mob  = lua->touserdata<fb::game::mob>(1);
    if (mob == nullptr)
        return 0;

    if (argc == 1)
    {
        auto result   = std::make_shared<bool>(false);
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *result = mob->hidden(*mob);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*result);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toboolean(2);
        auto weak     = mob->weak_from_this_as<fb::game::mob>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            mob->hidden(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}
