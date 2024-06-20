#include <fb/game/model.h>

uint16_t fb::model::map::width() const
{
    return this->_width;
}

uint16_t fb::model::map::height() const
{
    return this->_height;
}

int fb::model::map::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::model::map>(1);
    if(map == nullptr)
        return 0;

    thread->pushstring(map->name);
    return 1;
}

int fb::model::map::builtin_width(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::model::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->pushinteger(map->width());
    return 1;
}

int fb::model::map::builtin_height(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::model::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->pushinteger(map->height());
    return 1;
}

int fb::model::map::builtin_area(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::model::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->pushinteger(map->width());
    thread->pushinteger(map->height());
    return 2;
}

fb::model::map* fb::model::__map::name2map(const std::string& name) const
{
    auto i = std::find_if(this->begin(), this->end(), 
        [&name](auto& pair)
        {
            return pair.second.name == name;
        });

    return i != this->end() ? i->second.get() : nullptr;
}