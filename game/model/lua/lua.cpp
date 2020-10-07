#include "model/lua/lua.h"
using namespace fb::game::lua;

main* main::_instance;

std::string lua_cp949(lua_State* lua, int i)
{
    auto x = lua_tostring(lua, i);
    if(x == nullptr)
        return std::string();

    return CP949(x, PLATFORM::Windows);
}

void lua_push_utf8(lua_State* lua, const std::string& v)
{
    lua_pushstring(lua, UTF8(v, PLATFORM::Windows).c_str());
}

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

fb::game::lua::luable::luable(uint32_t id)
{
}

fb::game::lua::luable::~luable()
{
}

state::state(lua_State* lua) : _lua(lua)
{
}

fb::game::lua::state::state(lua_State* lua, const char* format, ...) : state(lua)
{
    va_list args;
    va_start(args, format);

    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
}

state::~state()
{
}

state& fb::game::lua::state::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
    return *this;
}

state& fb::game::lua::state::func(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    lua_getglobal(*this, buffer);
    return *this;
}

state& fb::game::lua::state::pushstring(const std::string& value)
{
    lua_pushstring(this->_lua, value.c_str());
    return *this;
}

state& fb::game::lua::state::pushinteger(int value)
{
    lua_pushinteger(this->_lua, value);
    return *this;
}

state& fb::game::lua::state::pushnil()
{
    lua_pushnil(this->_lua);
    return *this;
}

state& fb::game::lua::state::pushboolean(bool value)
{
    lua_pushboolean(this->_lua, value);
    return *this;
}

state& fb::game::lua::state::pushobject(const luable* object)
{
    object->to_lua(*this);
    return *this;
}

state& fb::game::lua::state::pushobject(const luable& object)
{
    object.to_lua(*this);
    return *this;
}

state::operator lua_State* () const
{
    return this->_lua;
}

thread::thread() : 
    state(lua_newthread(main::get())),
    _ref(luaL_ref(main::get(), LUA_REGISTRYINDEX))
{ }

fb::game::lua::thread::thread(const char* format, ...) : thread()
{
    va_list args;
    va_start(args, format);

    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
}

thread::~thread()
{
    luaL_unref(main::get(), LUA_REGISTRYINDEX, this->_ref);
}

thread& fb::game::lua::thread::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
    return *this;
}

thread& fb::game::lua::thread::func(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    lua_getglobal(*this, buffer);
    return *this;
}

thread& fb::game::lua::thread::pushstring(const std::string& value)
{
    return static_cast<thread&>(state::pushstring(value));
}

thread& fb::game::lua::thread::pushinteger(int value)
{
    return static_cast<thread&>(state::pushinteger(value));
}

thread& fb::game::lua::thread::pushnil()
{
    return static_cast<thread&>(state::pushnil());
}

thread& fb::game::lua::thread::pushboolean(bool value)
{
    return static_cast<thread&>(state::pushboolean(value));
}

thread& fb::game::lua::thread::pushobject(const luable* object)
{
    return static_cast<thread&>(state::pushobject(object));
}

thread& fb::game::lua::thread::pushobject(const luable& object)
{
    return static_cast<thread&>(state::pushobject(object));
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