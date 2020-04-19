#include "lua.h"
using namespace fb::game::lua;

main* main::_instance;

void fb::game::lua::luable::to_lua(lua_State* lua) const
{
    auto allocated = (void**)lua_newuserdata(lua, sizeof(void**));
    *allocated = (void*)this;

    auto metaname = this->metaname();
    luaL_getmetatable(lua, metaname.c_str());
    lua_setmetatable(lua, -2);
}

fb::game::lua::luable::luable()
{
}

fb::game::lua::luable::luable(uint32_t id) : base(id)
{
}

fb::game::lua::luable::~luable()
{
}

state::state(lua_State* lua) : _lua(lua)
{
}

state::~state()
{
}

void fb::game::lua::state::pushobject(const fb::game::lua::luable* object)
{
    object->to_lua(*this);
}

void fb::game::lua::state::pushobject(const fb::game::lua::luable& object)
{
    object.to_lua(*this);
}

void fb::game::lua::state::fromfile(const std::string& file, const std::string& name)
{
    luaL_dofile(*this, file.c_str());
    lua_getglobal(*this, name.c_str());
}

state::operator lua_State* () const
{
    return this->_lua;
}

thread::thread() : 
    state(lua_newthread(main::get())),
    _ref(luaL_ref(main::get(), LUA_REGISTRYINDEX))
{ }

thread::~thread()
{
    luaL_unref(main::get(), LUA_REGISTRYINDEX, this->_ref);
}

main::main() : state(luaL_newstate())
{
    luaL_openlibs(*this);
}

main::~main()
{
    lua_close(*this);
}

main& main::get()
{
    if(main::_instance == nullptr)
        main::_instance = new main();

    return *_instance;
}

void main::release()
{
    if(main::_instance == nullptr)
        return;
    
    delete main::_instance;
    main::_instance = nullptr;
}

void fb::game::lua::release()
{
    main::release();
}