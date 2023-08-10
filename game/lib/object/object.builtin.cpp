#include <fb/game/object.h>
#include <fb/game/map.h>
#include <fb/game/session.h>
#include <fb/game/mob.h>
#include <fb/game/context.h>
#include <fb/game/lua.builtin.h>

int fb::game::object::master::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object::master>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushstring(object->name);
    return 1;
}

int fb::game::object::master::builtin_look(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object::master>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->look);
    return 1;
}

int fb::game::object::master::builtin_color(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object::master>(1);
    if(object == nullptr)
        return 0;
    

    thread->pushinteger(object->color);
    return 1;
}

int fb::game::object::master::builtin_dialog(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    // Ex) npc:dialog(session, "hello", true, true);
    return ::builtin_dialog<object::master>(lua);
}

int fb::game::object::builtin_core(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto master = object->based();
    master->to_lua(lua);
    return 1;
}

int fb::game::object::builtin_id(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    thread->pushinteger(object->sequence());
    return 1;
}

int fb::game::object::builtin_eq(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto me = thread->touserdata<fb::game::object>(1);
    if(me == nullptr || context->exists(*me) == false)
        return 0;
    
    auto you = thread->touserdata<fb::game::object>(2);
    if(you == nullptr || context->exists(*you) == false)
        return 0;

    
    thread->pushboolean(me->sequence() == you->sequence());
    return 1;
}

int fb::game::object::builtin_tostring(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto me = thread->touserdata<fb::game::object>(1);
    if(me == nullptr || context->exists(*me) == false)
        return 0;
    
    thread->pushstring(me->name());
    return 1;
}

int fb::game::object::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    thread->pushstring(object->name());
    return 1;
}

int fb::game::object::builtin_dialog(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    return ::builtin_dialog<object>(lua);
}

int fb::game::object::builtin_sound(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto sound = thread->tointeger(2);

    context->send(*object, fb::protocol::game::response::object::sound(*object, sound::type(sound)), context::scope::PIVOT);
    thread->pushinteger(-1);
    return 1;
}

int fb::game::object::builtin_position(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    

    if(argc == 1)
    {
        thread->pushinteger(object->_position.x);
        thread->pushinteger(object->_position.y);
        return 2;
    }
    

    uint16_t x, y;
    if(thread->is_table(2))
    {
        thread->rawgeti(2, 1);
        x = (uint16_t)thread->tointeger(-1);
        thread->remove(-1);

        thread->rawgeti(2, 2);
        y = (uint16_t)thread->tointeger(-1);
        thread->remove(-1);
    }
    else
    {
        x = (uint16_t)thread->tointeger(2);
        y = (uint16_t)thread->tointeger(3);
    }

    std::vector<fb::game::object*> shows, hides, showings, hiddens;
    object->position(x, y, true);

    if(object->is(fb::game::object::types::SESSION))
    {
        context->send
        (
            *object, 
            [object](const auto& to)
            {
                return std::unique_ptr<fb::protocol::base::header>(new fb::protocol::game::response::session::show(static_cast<fb::game::session&>(*object), to));
            }, 
            context::scope::PIVOT
        );
    }
    else
    {
        context->send(*object, fb::protocol::game::response::object::show(*object), context::scope::PIVOT);
    }

    if(object->is(object::types::SESSION))
    {
        auto session = static_cast<fb::game::session*>(object);
        context->send(*object, fb::protocol::game::response::session::position(*session), context::scope::SELF);
    }

    return 0;
}

int fb::game::object::builtin_direction(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    

    if(argc == 1)
    {
        thread->pushinteger(object->_direction);
        return 1;
    }
    else
    {
        auto direction = fb::game::direction(thread->tointeger(2));
        object->direction(direction);

        auto context = thread->env<fb::game::context>("context");
        context->send(*object, fb::protocol::game::response::object::direction(*object), context::scope::PIVOT);
        return 0;
    }
}

int fb::game::object::builtin_chat(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto message = thread->tostring(2);
    auto type = argc < 3 ? chat::type::NORMAL : chat::type(thread->tointeger(3));
    auto decorate = argc < 4 ? true : thread->toboolean(4);

    std::stringstream sstream;
    if(decorate)
    {
        if(type == chat::type::SHOUT)
            sstream << object->name() << "! " << message;
        else
            sstream << object->name() << ": " << message;
    }
    else
    {
        sstream << message;
    }

    context->send(*object, fb::protocol::game::response::object::chat(*object, type, sstream.str()), context::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_message(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto message = thread->tostring(2);
    auto type = argc < 3 ? static_cast<int>(fb::game::message::type::STATE) : thread->tointeger(3);

    if(object->type() == object::types::SESSION)
        context->send(*object, fb::protocol::game::response::message(message, fb::game::message::type(type)), context::scope::SELF);

    return 0;
}

int fb::game::object::builtin_buff(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto spell = thread->touserdata<fb::game::spell>(2);
    if(spell == nullptr)
        return 0;

    auto seconds = (uint32_t)thread->tointeger(3);
    auto buff = object->buffs.push_back(spell, seconds);
    if(buff == nullptr)
        thread->pushnil();
    else
        context->send(*object, fb::protocol::game::response::spell::buff(*buff), context::scope::SELF);

    return 1;
}

int fb::game::object::builtin_unbuff(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    

    if(thread->is_str(2))
    {
        auto buff_name = thread->tostring(2);
        thread->pushboolean(object->buffs.remove(buff_name));
    }
    else if(thread->is_obj(2))
    {
        auto buff = thread->touserdata<fb::game::spell>(2);
        if(buff == nullptr)
            return 0;
        
        thread->pushboolean(object->buffs.remove(buff->name));
    }
    else
    {
        thread->pushboolean(false);
    }

    return 1;
}

int fb::game::object::builtin_isbuff(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    if(thread->is_str(2))
    {
        auto buff_name = thread->tostring(2);
        thread->pushboolean(object->buffs.contains(buff_name));
    }
    else if(thread->is_obj(2))
    {
        auto buff = thread->touserdata<fb::game::spell>(2);
        if(buff == nullptr)
            return 0;
        
        thread->pushboolean(object->buffs.contains(buff));
    }
    else
    {
        thread->pushboolean(false);
    }
    return 1;
}

int fb::game::object::builtin_effect(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto effect = (uint8_t)thread->tointeger(2);

    if(object->type() != object::types::ITEM)
        context->send(*object, fb::protocol::game::response::object::effect(*object, effect), context::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_map(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    try
    {
        auto argc = thread->argc();
        auto object = thread->touserdata<fb::game::object>(1);
        if(object == nullptr || context->exists(*object) == false)
            return 0;
        

        if(argc == 1)
        {
            auto map = object->map();
            if(map == nullptr)
                thread->pushnil();
            else
                map->to_lua(lua);
            return 1;
        }

        fb::game::map* map = nullptr;
        if(thread->is_obj(2))
        {
            map = thread->touserdata<fb::game::map>(2);
            if(map == nullptr)
                return 0;
            
            if(map == nullptr)
                throw std::exception();
        }
        else if(thread->is_str(2))
        {
            fb::game::data_set::maps.name2map(thread->tostring(2));
            if(map == nullptr)
                throw std::exception();
        }
        else
        {
            throw std::exception();
        }

        point16_t position;
        if(thread->is_table(3))
        {
            thread->rawgeti(3, 1);
            position.x = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);

            thread->rawgeti(3, 2);
            position.y = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);
        }
        else if(thread->is_num(3) && thread->is_num(4))
        {
            position.x = (uint16_t)thread->tointeger(3);
            position.y = (uint16_t)thread->tointeger(4);
        }
        else
        {
            throw std::exception();
        }

        object->map(map, position);
    }
    catch(...)
    { }

    return 0;
}

int fb::game::object::builtin_mkitem(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto name = thread->tostring(2);

    auto master = fb::game::data_set::items.name2item(name);
    if(master == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        auto context = thread->env<fb::game::context>("context");
        auto item = master->make(*context);
        item->map(object->_map, object->_position);
        item->to_lua(lua);
        
        context->send(*item, fb::protocol::game::response::object::show(*item), context::scope::PIVOT);
    }

    return 1;
}

int fb::game::object::builtin_showings(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto filter = argc < 2 ? object::types::UNKNOWN : object::types(thread->tointeger(2));

    thread->new_table();
    const auto& objects = object->showns(filter);

    for(int i = 0; i < objects.size(); i++)
    {
        objects[i]->to_lua(lua);
        thread->rawseti(-2, uint64_t(i+1));
    }

    return 1;
}

int fb::game::object::builtin_showns(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto filter = argc < 2 ? object::types::UNKNOWN : object::types(thread->tointeger(2));

    thread->new_table();
    const auto& objects = object->showings(filter);

    for(int i = 0; i < objects.size(); i++)
    {
        objects[i]->to_lua(lua);
        thread->rawseti(-2, uint64_t(i+1));
    }

    return 1;
}

int fb::game::object::builtin_front(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto filter = argc < 2 ? object::types::UNKNOWN : object::types(thread->tointeger(2));
    auto front = object->forward(filter);
    if(front == nullptr)
        thread->pushnil();
    else
        front->to_lua(lua);

    return 1;
}

int fb::game::object::builtin_is(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr || context->exists(*object) == false)
        return 0;
    
    auto type = thread->tointeger(2);
    thread->pushboolean(object->is(object::types(type)));
    return 1;
}