#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>
#include <tuple>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::item, "fb.model.item")
{"make",                builtin::model::item::builtin_make},
{"attr",                builtin::model::item::builtin_attr},
{"capacity",            builtin::model::item::builtin_capacity},
{"durability",          builtin::model::item::builtin_durability},
{"price",               builtin::model::item::builtin_price},
{"repair_price",        builtin::model::item::builtin_repair_price},
{"rename_price",        builtin::model::item::builtin_rename_price},
{"storage_fee",         builtin::model::item::builtin_storage_fee},
{"trade",               builtin::model::item::builtin_trade},
END_LUA_EXTENSION; // clang-format on

int builtin::model::item::builtin_make(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto& srv    = static_cast<fb::game::server&>(lua->executor);
    auto  model  = lua->touserdata<fb::model::item>(1);
    auto  object = model->make(srv);

    auto map = lua->touserdata<fb::game::map>(2);

    auto x = uint16_t{0};
    auto y = uint16_t{0};
    if (lua->is_table(3))
    {
        lua->rawgeti(3, 1);
        x = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);

        lua->rawgeti(3, 2);
        y = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);
    }
    else
    {
        x = (uint16_t)lua->tointeger(3);
        y = (uint16_t)lua->tointeger(4);
    }

    auto object_holder = std::make_shared<decltype(object)>();
    auto weak          = object->weak_from_this_as<fb::game::object>();
    auto builder       = lua->new_co_builder();
    builder.weak       = weak;
    builder.yield      = [=]() -> async::task<void> {
        std::ignore    = co_await object->map(map);
        std::ignore    = co_await object->position(x, y);
        *object_holder = object;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*object_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*object_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::model::item::builtin_attr(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::item>(1);
    auto flag  = (ITEM_ATTRIBUTE)lua->tointeger(2);

    lua->pushboolean(model->attr(flag));
    return 1;
}

int builtin::model::item::builtin_capacity(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::item>(1);
    lua->pushinteger(model->capacity);
    return 1;
}

int builtin::model::item::builtin_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto model = lua->touserdata<fb::model::item>(1);
    lua->pushinteger(model->price);
    return 1;
}

int builtin::model::item::builtin_durability(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model      = lua->touserdata<fb::model::item>(1);
    auto durability = uint32_t(0);
    if (model->attr(ITEM_ATTRIBUTE::PACK))
    {
        durability = static_cast<fb::model::pack*>(model)->durability;
    }
    else if (model->attr(ITEM_ATTRIBUTE::EQUIPMENT))
    {
        durability = static_cast<fb::model::equipment*>(model)->durability;
    }
    else
    {
        durability = 0;
    }

    lua->pushinteger(durability);
    return 1;
}

int builtin::model::item::builtin_repair_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::item>(1);
    if (model->attr(ITEM_ATTRIBUTE::EQUIPMENT))
    {
        auto repair_price = static_cast<fb::model::equipment*>(model)->repair;
        if (repair_price.has_value())
            lua->pushnumber(repair_price.value());
        else
            lua->pushnil();
    }
    else
    {
        lua->pushnil();
    }

    return 1;
}

int builtin::model::item::builtin_rename_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::item>(1);
    if (model->attr(ITEM_ATTRIBUTE::WEAPON))
    {
        auto rename_price = static_cast<fb::model::weapon*>(model)->rename;
        if (rename_price.has_value())
            lua->pushinteger(rename_price.value());
        else
            lua->pushnil();
    }
    else
    {
        lua->pushnil();
    }

    return 1;
}

int builtin::model::item::builtin_storage_fee(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::item>(1);
    if (model->storage_fee.has_value())
    {
        lua->pushinteger(model->storage_fee.value());
    }
    else
    {
        lua->pushnil();
    }
    return 1;
}

int builtin::model::item::builtin_trade(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::item>(1);
    lua->pushboolean(model->trade);
    return 1;
}
