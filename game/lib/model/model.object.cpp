#include <model.h>
#include <object.h>
#include <map.h>
#include <character.h>
#include <mob.h>
#include <context.h>

uint8_t fb::model::object::dialog_look_type() const
{
    return this->look > 0xBFFF ? 0x02 : 0x01;
}

fb::model::enum_value::OBJECT_TYPE fb::model::object::what() const
{
    return fb::model::enum_value::OBJECT_TYPE::UNKNOWN;
}

bool fb::model::object::operator == (const fb::model::object& r) const
{
    return this == &r;
}

bool fb::model::object::operator != (const fb::model::object& r) const
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
    // Ex) npc:dialog(session, "hello", true, true);
    
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    try
    {
        auto argc = thread->argc();
        if(argc < 3)
            throw std::runtime_error("not enough parameters");

        auto context = thread->env<fb::game::context>("context");
        auto object = thread->touserdata<fb::model::object>(1);
        if(object == nullptr)
            return 0;

        auto session = thread->touserdata<fb::game::character>(2);
        if(session == nullptr)
            return 0;

        auto message = thread->tostring(3);
        auto button_prev = argc < 4 ? false : thread->toboolean(4);
        auto button_next = argc < 5 ? false : thread->toboolean(5);

        session->dialog.show(*object, message, button_prev, button_next);
        return thread->yield(1);
    }
    catch(std::exception&)
    {
        thread->pushnil();
        return 1;
    }
}