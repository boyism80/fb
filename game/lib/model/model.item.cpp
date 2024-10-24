#include <context.h>
#include <item.h>
#include <model.h>

ITEM_ATTRIBUTE fb::model::item::attr() const
{
    auto attr = ITEM_ATTRIBUTE::NONE;
    if (this->capacity > 1)
        attr = ITEM_ATTRIBUTE((uint32_t)attr | (uint32_t)ITEM_ATTRIBUTE::BUNDLE);
    return attr;
}

bool fb::model::item::attr(ITEM_ATTRIBUTE flag) const
{
    return ((uint32_t)this->attr() & (uint32_t)flag) == (uint32_t)flag;
}

fb::model::item* fb::model::__item::name2item(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }

    return nullptr;
}

int fb::model::item::builtin_make(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto model   = thread->touserdata<fb::model::item>(1);
    auto object  = model->make(*context);

    auto map     = thread->touserdata<fb::game::map>(2);
    async::awaitable_get(object->map(map));

    if (lua_istable(lua, 3))
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

int fb::model::item::builtin_attr(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto model = thread->touserdata<fb::model::item>(1);
    auto flag  = (ITEM_ATTRIBUTE)thread->tointeger(2);

    thread->pushboolean(model->attr(flag));
    return 1;
}

int fb::model::item::builtin_capacity(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto model = thread->touserdata<fb::model::item>(1);
    thread->pushinteger(model->capacity);
    return 1;
}

int fb::model::item::builtin_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto model = thread->touserdata<fb::model::item>(1);
    thread->pushinteger(model->price);
    return 1;
}

int fb::model::item::builtin_durability(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto model      = thread->touserdata<fb::model::item>(1);
    auto durability = uint16_t(0);
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

    thread->pushinteger(durability);
    return 1;
}

int fb::model::item::builtin_repair_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto model = thread->touserdata<fb::model::item>(1);
    if (model->attr(ITEM_ATTRIBUTE::EQUIPMENT))
    {
        auto repair_price = static_cast<fb::model::equipment*>(model)->repair;
        if (repair_price.has_value())
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

int fb::model::item::builtin_rename_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto model = thread->touserdata<fb::model::item>(1);
    if (model->attr(ITEM_ATTRIBUTE::WEAPON))
    {
        auto rename_price = static_cast<fb::model::weapon*>(model)->rename;
        if (rename_price.has_value())
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

int fb::model::item::builtin_deposit_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto model = thread->touserdata<fb::model::item>(1);
    if (model->deposit_price.has_value())
    {
        thread->pushinteger(model->deposit_price.value());
    }
    else
    {
        thread->pushnil();
    }
    return 1;
}

fb::game::item* fb::model::item::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::item>(*this, fb::game::item::config{.count = count});
}

fb::game::item* fb::model::cash::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::cash>(count);
}

fb::game::item* fb::model::consume::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::consume>(*this, count);
}

fb::game::item* fb::model::pack::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::pack>(*this);
}

fb::game::item* fb::model::weapon::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::weapon>(*this);
}

fb::game::item* fb::model::armor::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::armor>(*this);
}

fb::game::item* fb::model::helmet::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::helmet>(*this);
}

fb::game::item* fb::model::shield::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::shield>(*this);
}

fb::game::item* fb::model::ring::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::ring>(*this);
}

fb::game::item* fb::model::auxiliary::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::auxiliary>(*this);
}

fb::game::item* fb::model::bow::make(fb::game::context& context, uint16_t count) const
{
    return context.make<fb::game::bow>(*this);
}