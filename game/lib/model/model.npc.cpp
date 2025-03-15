#include <fb/model/model.h>
#include <fb/game/context.h>

int fb::model::npc::builtin_sell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
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

int fb::model::npc::builtin_sell_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto name = lua->tostring(2);
    for (auto sell : npc->sell)
    {
        if (context->model.sell.contains(sell) == false)
            continue;

        for (auto& [_, x] : context->model.sell[sell])
        {
            auto& item = context->model.item[x.item];
            if (item.name != name)
                continue;

            lua->pushinteger(x.price.value_or(item.price));
            return 1;
        }
    }

    return 0;
}

int fb::model::npc::builtin_buy_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto name = lua->tostring(2);
    if (npc->buy.has_value() == false)
        return 0;

    if (context->model.buy.contains(npc->buy.value()) == false)
        return 0;

    for (auto& [_, x] : context->model.buy[npc->buy.value()])
    {
        auto& item = context->model.item[x.item];
        if (item.name != name)
            continue;

        lua->pushinteger(x.price.value_or(item.price / 2));
        return 1;
    }

    return 0;
}

int fb::model::npc::builtin_buy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto npc     = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    if (npc->buy.has_value())
        lua->pushinteger(npc->buy.value());
    else
        lua->pushnil();

    return 1;
}

fb::model::npc* fb::model::__npc::name2npc(const std::string& name) const
{
    for (auto& npc : *this)
    {
        if (npc.second.name == name)
            return &npc.second;
    }

    return nullptr;
}