#include <fb/game/context.h>
#include <fb/game/item.h>
#include <fb/model/model.h>

int fb::model::weapon::builtin_damage_small(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto model   = thread->touserdata<fb::model::weapon>(1);

    thread->pushinteger(model->damage_small.min);
    thread->pushinteger(model->damage_small.max);
    return 2;
}

int fb::model::weapon::builtin_damage_large(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto model   = thread->touserdata<fb::model::weapon>(1);

    thread->pushinteger(model->damage_large.min);
    thread->pushinteger(model->damage_large.max);
    return 2;
}

int fb::model::weapon::builtin_sound(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto model   = thread->touserdata<fb::model::weapon>(1);

    thread->pushinteger(model->sound);
    return 1;
}

int fb::model::weapon::builtin_type(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto model   = thread->touserdata<fb::model::weapon>(1);

    thread->pushinteger(model->weapon_type());
    return 1;
}