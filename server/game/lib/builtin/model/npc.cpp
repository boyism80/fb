#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::npc, "fb.model.npc")
{"sell",                builtin::model::npc::builtin_sell},
{"sell_price",          builtin::model::npc::builtin_sell_price},
{"buy",                 builtin::model::npc::builtin_buy},
{"buy_price",           builtin::model::npc::builtin_buy_price},
END_LUA_EXTENSION; // clang-format on

int builtin::model::npc::builtin_sell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto npc    = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    lua->new_table();
    for (int i = 0; i < npc->sell.size(); i++)
    {
        lua->pushinteger(i + 1);
        lua->pushinteger(npc->sell[i]);
        lua_settable(L, -3);
    }
    return 1;
}

int builtin::model::npc::builtin_sell_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto npc    = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto name = lua->tostring(2);
    for (auto sell : npc->sell)
    {
        if (table::sell.contains(sell) == false)
            continue;

        for (auto& x : table::sell[sell])
        {
            auto& item = table::item[x.item];
            if (item.name != name)
                continue;

            lua->pushinteger(x.price.value_or(item.price));
            return 1;
        }
    }

    return 0;
}

int builtin::model::npc::builtin_buy_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto npc    = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto name = lua->tostring(2);
    if (npc->buy.has_value() == false)
        return 0;

    if (table::buy.contains(npc->buy.value()) == false)
        return 0;

    for (auto& [_, x] : table::buy[npc->buy.value()])
    {
        auto& item = table::item[x.item];
        if (item.name != name)
            continue;

        lua->pushinteger(x.price.value_or(item.price / 2));
        return 1;
    }

    return 0;
}

int builtin::model::npc::builtin_buy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto npc    = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    if (npc->buy.has_value())
        lua->pushinteger(npc->buy.value());
    else
        lua->pushnil();

    return 1;
}