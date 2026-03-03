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
{"interaction",         builtin::model::npc::builtin_interaction},
{"appearance",          builtin::model::npc::builtin_appearance},
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

int builtin::model::npc::builtin_interaction(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto npc = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    lua->pushinteger(static_cast<uint8_t>(npc->interaction));
    return 1;
}

int builtin::model::npc::builtin_appearance(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto npc = lua->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    if (!npc->appearance.has_value() || !table::appearance.contains(npc->appearance.value()))
    {
        lua->new_table();
        lua->pushstring("disguise");
        lua->pushinteger(static_cast<lua_Integer>(npc->look));
        lua->settable(-3);
        lua->pushstring("hair_color");
        lua->pushinteger(static_cast<lua_Integer>(npc->color));
        lua->settable(-3);
        return 1;
    }

    auto const& p = table::appearance[npc->appearance.value()];
    lua->new_table();
    lua->pushstring("gender");
    lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(p.gender)));
    lua->settable(-3);
    lua->pushstring("state");
    lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(p.state)));
    lua->settable(-3);
    lua->pushstring("hair");
    lua->pushinteger(static_cast<lua_Integer>(p.hair));
    lua->settable(-3);
    lua->pushstring("hair_color");
    lua->pushinteger(static_cast<lua_Integer>(p.hair_color));
    lua->settable(-3);
    if (p.weapon.has_value())
    {
        lua->pushstring("weapon");
        lua->pushinteger(static_cast<lua_Integer>(p.weapon.value()));
        lua->settable(-3);
    }
    if (p.weapon_color.has_value())
    {
        lua->pushstring("weapon_color");
        lua->pushinteger(static_cast<lua_Integer>(p.weapon_color.value()));
        lua->settable(-3);
    }
    if (p.armor.has_value())
    {
        lua->pushstring("armor");
        lua->pushinteger(static_cast<lua_Integer>(p.armor.value()));
        lua->settable(-3);
    }
    if (p.armor_color.has_value())
    {
        lua->pushstring("armor_color");
        lua->pushinteger(static_cast<lua_Integer>(p.armor_color.value()));
        lua->settable(-3);
    }
    if (p.shield.has_value())
    {
        lua->pushstring("shield");
        lua->pushinteger(static_cast<lua_Integer>(p.shield.value()));
        lua->settable(-3);
    }
    if (p.shield_color.has_value())
    {
        lua->pushstring("shield_color");
        lua->pushinteger(static_cast<lua_Integer>(p.shield_color.value()));
        lua->settable(-3);
    }
    if (p.disguise.has_value())
    {
        lua->pushstring("disguise");
        lua->pushinteger(static_cast<lua_Integer>(p.disguise.value()));
        lua->settable(-3);
    }
    return 1;
}