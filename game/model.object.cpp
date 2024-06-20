#include <fb/game/model.h>
#include <fb/game/object.h>
#include <fb/game/map.h>
#include <fb/game/session.h>
#include <fb/game/mob.h>
#include <fb/game/context.h>
#include <fb/game/built_in.h>

uint8_t fb::model::object::dialog_look_type() const
{
    return this->look > 0xBFFF ? 0x02 : 0x01;
}

fb::model::enum_value::OBJECT_TYPE fb::model::object::type() const
{
    return fb::model::enum_value::OBJECT_TYPE::UNKNOWN;
}

bool fb::model::object::operator == (const fb::model::object& r) const
{
    return this == &r;
}

bool fb::model::object::operator == (const fb::model::object& r) const
{
    return this != &r;
}

int fb::model::object::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::model::object>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushstring(object->name);
    return 1;
}

int fb::model::object::builtin_look(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::model::object>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->look);
    return 1;
}

int fb::model::object::builtin_color(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::model::object>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->color);
    return 1;
}

int fb::model::object::builtin_dialog(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    // Ex) npc:dialog(session, "hello", true, true);
    return ::builtin_dialog<fb::model::object>(lua);
}