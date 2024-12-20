#include <fb/model/model.h>

int fb::model::trace::builtin_id(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto trace   = thread->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    thread->pushinteger(trace->id);
    return 1;
}

int fb::model::trace::builtin_look(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto trace   = thread->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    thread->pushinteger(trace->look);
    return 1;
}

int fb::model::trace::builtin_color(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto trace   = thread->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    thread->pushinteger(trace->color);
    return 1;
}

int fb::model::trace::builtin_text(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto trace   = thread->touserdata<fb::model::trace>(1);
    if (trace == nullptr)
        return 0;

    thread->pushstring(trace->text);
    return 1;
}