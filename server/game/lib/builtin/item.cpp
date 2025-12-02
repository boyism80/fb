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

    auto  item  = lua->touserdata<fb::game::item>(1);
    auto& model = item->based<fb::model::item>();

    lua->pushobject(model);
    return 1;
}

int builtin::item::builtin_count(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto item = lua->touserdata<fb::game::item>(1);
    lua->pushinteger(item->count());
    return 1;
}

int builtin::item::builtin_durability(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto item = lua->touserdata<fb::game::item>(1);

    if (argc > 1)
    {
        auto value = lua->tointeger(2);
        item->durability(value);
        return 0;
    }
    else
    {
        auto durability = item->durability();
        if (durability.has_value())
            lua->pushinteger(durability.value());
        else
            lua->pushnil();
        return 1;
    }
}

int builtin::item::builtin_rename(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto  argc   = lua->argc();
    auto  item   = lua->touserdata<fb::game::item>(1);
    auto& model  = item->based<fb::model::item>();
    auto  weapon = model.attr(ITEM_ATTRIBUTE::WEAPON) ? item->weak_from_this_as<fb::game::weapon>() : std::weak_ptr<fb::game::weapon>();

    if (weapon.expired())
    {
        lua->pushboolean(false);
        return 1;
    }

    if (argc > 1)
    {
        if (lua->is_string(2))
        {
            auto name = lua->tostring(2);
            weapon.lock()->custom_name(name);
            return 0;
        }
        else if (lua->is_nil(2))
        {
            weapon.lock()->reset_custom_name();
            return 0;
        }
        else
        {
            lua->pushboolean(false);
            return 1;
        }
    }
    else
    {
        auto& custom_name = weapon.lock()->custom_name();
        if (custom_name.has_value())
            lua->pushstring(custom_name.value());
        else
            lua->pushnil();
        return 1;
    }
}