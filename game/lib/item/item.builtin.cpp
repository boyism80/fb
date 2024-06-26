#include <fb/game/item.h>
#include <fb/game/context.h>


int fb::game::item::builtin_model(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto item = thread->touserdata<fb::game::item>(1);
    auto& model = item->based<fb::model::item>();

    thread->pushobject(model);
    return 1;
}

int fb::game::item::builtin_count(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto item = thread->touserdata<fb::game::item>(1);
    thread->pushinteger(item->count());
    return 1;
}

int fb::game::item::builtin_durability(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto item = thread->touserdata<fb::game::item>(1);

    if(argc > 1)
    {
        auto value = thread->tointeger(2);
        item->durability(value);
        return 0;
    }
    else
    {
        auto durability = item->durability();
        if(durability.has_value())
            thread->pushinteger(durability.value());
        else
            thread->pushnil();
        return 1;
    }
}

int fb::game::item::builtin_rename(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto item = thread->touserdata<fb::game::item>(1);
    auto& model = item->based<fb::model::item>();
    auto weapon = model.attr(ITEM_ATTRIBUTE::WEAPON) ? static_cast<fb::game::weapon*>(item) : nullptr;

    if (weapon == nullptr)
    {
        thread->pushboolean(false);
        return 1;
    }

    if(argc > 1)
    {
        if (thread->is_str(2))
        {
            auto name = thread->tostring(2);
            weapon->custom_name(name);
            return 0;
        }
        else if (thread->is_nil(2))
        {
            weapon->reset_custom_name();
            return 0;
        }
        else
        {
            thread->pushboolean(false);
            return 1;
        }
    }
    else
    {
        auto& custom_name = weapon->custom_name();
        if (custom_name.has_value())
            thread->pushstring(custom_name.value());
        else
            thread->pushnil();
        return 1;
    }
}