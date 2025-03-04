#include <fb/game/context.h>
#include <fb/game/item.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::item, "fb.game.item")
{"model",               fb::game::item::builtin_model},
{"count",               fb::game::item::builtin_count},
{"durability",          fb::game::item::builtin_durability},
{"rename",              fb::game::item::builtin_rename},
END_LUA_EXTENSION; // clang-format on

int fb::game::item::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto  item  = lua->touserdata<fb::game::item>(1);
    auto& model = item->based<fb::model::item>();

    lua->pushobject(model);
    return 1;
}

int fb::game::item::builtin_count(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto item = lua->touserdata<fb::game::item>(1);
    lua->pushinteger(item->count());
    return 1;
}

int fb::game::item::builtin_durability(lua_State* L)
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

int fb::game::item::builtin_rename(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto  argc   = lua->argc();
    auto  item   = lua->touserdata<fb::game::item>(1);
    auto& model  = item->based<fb::model::item>();
    auto  weapon = model.attr(ITEM_ATTRIBUTE::WEAPON) ? static_cast<fb::game::weapon*>(item) : nullptr;

    if (weapon == nullptr)
    {
        lua->pushboolean(false);
        return 1;
    }

    if (argc > 1)
    {
        if (lua->is_str(2))
        {
            auto name = lua->tostring(2);
            weapon->custom_name(name);
            return 0;
        }
        else if (lua->is_nil(2))
        {
            weapon->reset_custom_name();
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
        auto& custom_name = weapon->custom_name();
        if (custom_name.has_value())
            lua->pushstring(custom_name.value());
        else
            lua->pushnil();
        return 1;
    }
}