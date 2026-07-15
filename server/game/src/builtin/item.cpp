#include <fb/game/server.h>
#include <fb/game/item.h>
#include <fb/game/builtin/item.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(item, "fb.game.item")
{"model",               builtin::item::builtin_model},
{"count",               builtin::item::builtin_count},
{"durability",          builtin::item::builtin_durability},
{"rename",              builtin::item::builtin_rename},
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
        *model_ptr = &item->based<fb::model::item>();
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

int builtin::item::builtin_rename(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto item = lua->touserdata<fb::game::item>(1);
    if (item == nullptr)
        return 0;

    auto& model  = item->based<fb::model::item>();
    auto  weapon = model.attr(ITEM_ATTRIBUTE::WEAPON) ? item->weak_from_this_as<fb::game::weapon>()
                                                      : std::weak_ptr<fb::game::weapon>();

    if (weapon.expired())
    {
        lua->pushboolean(false);
        return 1;
    }

    if (argc > 1)
    {
        auto builder = lua->new_co_builder();
        builder.weak = weapon;
        if (lua->is_string(2))
        {
            auto name     = lua->tostring(2);
            builder.yield = [=]() -> async::task<void> {
                weapon.lock()->custom_name(name);
                co_return;
            };
            builder.resume = []() -> async::task<int> {
                co_return 0;
            };
        }
        else if (lua->is_nil(2))
        {
            builder.yield = [=]() -> async::task<void> {
                weapon.lock()->reset_custom_name();
                co_return;
            };
            builder.resume = []() -> async::task<int> {
                co_return 0;
            };
        }
        else
        {
            lua->pushboolean(false);
            return 1;
        }
        return builder.run();
    }
    else
    {
        auto custom_name = std::make_shared<std::optional<std::string>>();
        auto builder     = lua->new_co_builder();
        builder.weak     = weapon;
        builder.yield    = [=]() -> async::task<void> {
            *custom_name = weapon.lock()->custom_name();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (custom_name->has_value())
                lua->pushstring(custom_name->value());
            else
                lua->pushnil();
            co_return 1;
        };
        return builder.run();
    }
}
