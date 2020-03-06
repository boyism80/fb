#ifndef __LUA_H__
#define __LUA_H__

extern "C"
{
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include <vector>
#include <string>

#define DECLARE_LUA_PROTOTYPE               static const struct luaL_Reg    LUA_METHODS[];\
                                            static const std::string        LUA_METATABLE_NAME;

#define IMPLEMENT_LUA_EXTENSION(type, name) const std::string type::LUA_METATABLE_NAME = name;\
                                            const struct luaL_Reg type::LUA_METHODS[] = \
                                            {

#define END_LUA_EXTENSION                   {NULL, NULL}\
                                            };


#define BUILTIN_CORE(T)                     static int builtin_core(lua_State* lua)\
                                            {\
                                                auto object = *(T**)lua_touserdata(lua, 1);\
                                                auto core = object->based<T::core>();\
                                                core->new_lua<T::core>(lua);\
                                                return 1;\
                                            }

#define IMPLEMENT_NEW_LUA                   template <typename T>\
                                            void new_lua(lua_State* lua) const\
                                            {\
                                                auto allocated = (const T**)lua_newuserdata(lua, sizeof(T**));\
                                                *allocated = static_cast<const T*>(this);\
                                                auto metaname = T::LUA_METATABLE_NAME.c_str();\
                                                luaL_getmetatable(lua, metaname);\
                                                auto ret = lua_setmetatable(lua, -2);\
                                            }

namespace fb { namespace game { namespace lua {

class state
{
private:
	lua_State*				_lua;

protected:
	state(lua_State* lua);
	virtual ~state();

public:
	void					pushstring(const std::string& value) { lua_pushstring(*this, value.c_str()); }
	void					pushinteger(int value) { lua_pushinteger(*this, value); }
	void					pushnil() { lua_pushnil(*this); }
	void					pushboolean(bool value) { lua_pushboolean(*this, value); }

	const std::string		tostring(int offset) { return lua_tostring(*this, offset); }
	const std::string		arg_string(int offset) { return tostring(offset); }
	const std::string		ret_string(int offset) { return tostring(-offset); }
	
	int						tointeger(int offset) { return lua_tointeger(*this, offset); }
	int						arg_integer(int offset) { return tointeger(offset); }
	int						ret_integer(int offset) { return tointeger(-offset); }
	
	bool					toboolean(int offset) { return lua_toboolean(*this, offset); }
	bool					arg_boolean(int offset) { return toboolean(offset); }
	bool					ret_boolean(int offset) { return toboolean(-offset); }

public:
	operator				lua_State* () const;
};

class main : public state
{
private:
	static main* _instance;

public:
	main();
	~main();

public:
	static main&			get();
	static void				release();
};

class thread : public state
{
private:
	int						_ref;

public:
	thread();
	~thread();

public:
	bool					resume(int num_args) { return lua_resume(*this, num_args) == 0; } // 종료되면 true, 아니면 false
	int						yield(int num_rets) { return lua_yield(*this, num_rets); }
};

void release();

template <typename T>
void bind_class()
{
    luaL_newmetatable(main::get(), T::LUA_METATABLE_NAME.c_str());

    lua_pushvalue(main::get(), -1);
    lua_setfield(main::get(), -2, "__index");
    luaL_register(main::get(), NULL, T::LUA_METHODS);
}

template <typename T, typename B>
void bind_class()
{
    luaL_newmetatable(main::get(), T::LUA_METATABLE_NAME.c_str());
    
    luaL_getmetatable(main::get(), B::LUA_METATABLE_NAME.c_str());
    lua_setmetatable(main::get(), -2);

    lua_pushvalue(main::get(), -1);
    lua_setfield(main::get(), -2, "__index");
    luaL_register(main::get(), NULL, T::LUA_METHODS);
}

template <typename T>
void env(const char* key, T* data)
{
	lua_pushstring(main::get(), key);
	lua_pushlightuserdata(main::get(), data);
	lua_settable(main::get(), LUA_REGISTRYINDEX);
}

template <typename T>
T* env(const char* key)
{
	lua_pushstring(main::get(), key);
	lua_gettable(main::get(), LUA_REGISTRYINDEX);

	return static_cast<T*>(lua_touserdata(main::get(), -1));
}


} } }

#endif // !__LUA_H__
