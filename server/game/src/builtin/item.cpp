#include <fb/game/builtin/item.h>

#include <fb/game/item.h>
#include <fb/game/server.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(item, "fb.game.item")
{"model",               builtin::item::builtin_model},
{"count",               builtin::item::builtin_count},
{"uid",                 builtin::item::builtin_uid},
{"durability",          builtin::item::builtin_durability},
END_LUA_EXTENSION; // clang-format on

int builtin::item::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto item = lua->touserdata<fb::game::item>(1);
    if (item == nullptr)
        return 0;

    auto model_ptr = std::make_shared<const fb::model::item*>();
    auto weak      = item->weak_from_this_as<fb::game::item>();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *model_ptr = &item->model();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushobject(**model_ptr);
        co_return 1;
    };
    return builder.run();
}

int builtin::item::builtin_count(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto item = lua->touserdata<fb::game::item>(1);
    if (item == nullptr)
        return 0;

    auto count    = std::make_shared<uint16_t>();
    auto weak     = item->weak_from_this_as<fb::game::item>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *count = item->count();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*count);
        co_return 1;
    };
    return builder.run();
}

int builtin::item::builtin_uid(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto item = lua->touserdata<fb::game::item>(1);
    if (item == nullptr)
        return 0;

    auto uid      = std::make_shared<uint64_t>();
    auto weak     = item->weak_from_this_as<fb::game::item>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *uid = item->uid();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*uid == 0)
            lua->pushnil();
        else
            lua->pushinteger(*uid);
        co_return 1;
    };
    return builder.run();
}

int builtin::item::builtin_durability(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto item = lua->touserdata<fb::game::item>(1);
    if (item == nullptr)
        return 0;

    auto weak    = item->weak_from_this_as<fb::game::item>();
    auto builder = lua->new_co_builder();

    if (argc > 1)
    {
        auto value    = lua->tointeger(2);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            item->durability(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else
    {
        auto durability = std::make_shared<std::optional<uint32_t>>();
        builder.weak    = weak;
        builder.yield   = [=]() -> async::task<void> {
            *durability = item->durability();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (durability->has_value())
                lua->pushinteger(durability->value());
            else
                lua->pushnil();
            co_return 1;
        };
        return builder.run();
    }
}