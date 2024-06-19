#include <fb/game/model.h>
#include <fb/game/item.h>

using namespace fb;

model::enum_value::ITEM_ATTRIBUTE model::item::attr() const
{
    auto                    attr = model::enum_value::ITEM_ATTRIBUTE::NONE;
    if(this->capacity > 1)
        attr = model::enum_value::ITEM_ATTRIBUTE((uint32_t)attr | (uint32_t)model::enum_value::ITEM_ATTRIBUTE::BUNDLE);
    return attr;
}

bool model::item::attr(model::enum_value::ITEM_ATTRIBUTE flag) const
{
    return ((uint32_t)this->attr() & (uint32_t)flag) == (uint32_t)flag;
}

int model::item::builtin_make(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto model = thread->touserdata<model::item>(1);
    auto object = model->make(*context);

    auto map = thread->touserdata<fb::game::map>(2);
    object->map(map);

    if(lua_istable(lua, 3))
    {
        lua_rawgeti(lua, 3, 1);
        object->x((uint16_t)thread->tointeger(-1));
        lua_remove(lua, -1);

        lua_rawgeti(lua, 3, 2);
        object->y((uint16_t)thread->tointeger(-1));
        lua_remove(lua, -1);
    }
    else
    {
        object->position((uint16_t)thread->tointeger(3), (uint16_t)thread->tointeger(4));
    }

    thread->pushobject(object);
    return 1;
}

int model::item::builtin_attr(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto model = thread->touserdata<model::item>(1);
    auto flag = (model::enum_value::ITEM_ATTRIBUTE)thread->tointeger(2);

    thread->pushboolean(model->attr(flag));
    return 1;
}

int model::item::builtin_capacity(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto model = thread->touserdata<model::item>(1);
    thread->pushinteger(model->capacity);
    return 1;
}

int model::item::builtin_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto model = thread->touserdata<model::item>(1);
    thread->pushinteger(model->price);
    return 1;
}

int model::item::builtin_durability(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto model = thread->touserdata<model::item>(1);
    auto durability = uint16_t(0);
    if(model->attr(model::enum_value::ITEM_ATTRIBUTE::PACK))
    {
        durability = static_cast<model::pack*>(model)->durability;
    }
    else if(model->attr(model::enum_value::ITEM_ATTRIBUTE::EQUIPMENT))
    {
        durability = static_cast<model::equipment*>(model)->durability;
    }
    else
    {
        durability = 0;
    }

    thread->pushinteger(durability);
    return 1;
}

int model::item::builtin_repair_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto model = thread->touserdata<model::item>(1);
    if(model->attr(model::enum_value::ITEM_ATTRIBUTE::EQUIPMENT))
    {
        auto repair_price = static_cast<model::equipment*>(model)->repair;
        if(repair_price.has_value())
            thread->pushnumber(repair_price.value());
        else
            thread->pushnil();
    }
    else
    {
        thread->pushnil();
    }

    return 1;
}

int model::item::builtin_rename_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto model = thread->touserdata<model::item>(1);
    if(model->attr(model::enum_value::ITEM_ATTRIBUTE::WEAPON))
    {
        auto rename_price = static_cast<model::weapon*>(model)->rename;
        if(rename_price.has_value())
            thread->pushinteger(rename_price.value());
        else
            thread->pushnil();
    }
    else
    {
        thread->pushnil();
    }

    return 1;
}

int model::item::builtin_deposit_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto model = thread->touserdata<model::item>(1);
    if(model->deposit_price.has_value())
    {
        thread->pushinteger(model->deposit_price.value());
    }
    else
    {
        thread->pushnil();
    }
    return 1;
}