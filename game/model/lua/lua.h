#ifndef __LUA_H__
#define __LUA_H__

extern "C"
{
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
}

#include <vector>
#include <string>
#include "module/socket/socket.h"
#include "module/encoding/encoding.h"

#define LUA_PROTOTYPE                       static const struct luaL_Reg    LUA_METHODS[];\
                                            static const std::string        LUA_METATABLE_NAME;\
                                            virtual const std::string& metaname() const { return this->LUA_METATABLE_NAME; }

#define IMPLEMENT_LUA_EXTENSION(type, name) const std::string type::LUA_METATABLE_NAME = name;\
                                            const struct luaL_Reg type::LUA_METHODS[] = \
                                            {

#define END_LUA_EXTENSION                   {NULL, NULL}\
                                            };

#define LUA_PENDING (LUA_ERRERR+1)


// global
std::string                     lua_cp949(lua_State* lua, int i);
void                            lua_push_utf8(lua_State* lua, const std::string& v);

namespace fb { namespace game { namespace lua {

class luable
{
private:
    std::vector<void**>         _pointers;

#pragma region lua
public:
    LUA_PROTOTYPE
    void                        to_lua(lua_State* lua) const;
#pragma endregion

#pragma region public method
protected:
    luable();
    luable(uint32_t id);

public:
    virtual ~luable();
#pragma endregion

public:
    static int                  builtin_gc(lua_State* lua);
};

class state
{
protected:
    lua_State*                  _lua;

protected:
    state(lua_State* lua);
    state(lua_State* lua, const char* format, ...);
    virtual ~state();

protected:

public:
    state&                      from(const char* format, ...);
    state&                      func(const char* format, ...);

    state&                      pushstring(const std::string& value);
    state&                      pushinteger(int value);
    state&                      pushnil();
    state&                      pushboolean(bool value);
    state&                      pushobject(const luable* object);
    state&                      pushobject(const luable& object);

    const std::string           tostring(int offset) { return lua_cp949(*this, offset); }
    const std::string           arg_string(int offset) { return tostring(offset); }
    const std::string           ret_string(int offset) { return tostring(-offset); }
    
    int                         tointeger(int offset) { return (int)lua_tointeger(*this, offset); }
    int                         arg_integer(int offset) { return tointeger(offset); }
    int                         ret_integer(int offset) { return tointeger(-offset); }
    
    bool                        toboolean(int offset) { return lua_toboolean(*this, offset); }
    bool                        arg_boolean(int offset) { return toboolean(offset); }
    bool                        ret_boolean(int offset) { return toboolean(-offset); }

    template <typename T>
    T*                          touserdata(int offset) { return *(T**)lua_touserdata(*this, offset); }

    bool                        is_str(int offset) { return lua_isstring(*this, offset); }
    bool                        is_obj(int offset) { return lua_isuserdata(*this, offset); }
    bool                        is_table(int offset) { return lua_istable(*this, offset); }
    bool                        is_num(int offset) { return lua_isnumber(*this, offset); }

    int                         rawgeti(int offset_t, int offset_e) { return lua_rawgeti(*this, offset_t, offset_e); }
    void                        rawseti(int offset_t, int offset_e) { lua_rawseti(*this, offset_t, offset_e); }
    int                         rawlen(int offset_t) { return lua_rawlen(*this, offset_t); }
    void                        remove(int offset) { lua_remove(*this, offset); }
    void                        new_table() { lua_newtable(*this); }


public:
    operator                    lua_State* () const;
};

class thread;

class main : public state
{
public:
    std::map<lua_State*, thread*> threads;

private:
    static main* _instance;

public:
    main();
    ~main();

public:
    static main&                get();
    static void                 release();
};

class thread : public state
{
private:
    int                         _state;
    int                         _ref;

public:
    thread();
    thread(const char* format, ...);
    ~thread();

public:
    thread&                     from(const char* format, ...);
    thread&                     func(const char* format, ...);

    thread&                     pushstring(const std::string& value);
    thread&                     pushinteger(int value);
    thread&                     pushnil();
    thread&                     pushboolean(bool value);
    thread&                     pushobject(const luable* object);
    thread&                     pushobject(const luable& object);

public:
    int                         argc() const;

public:
    int                         resume(int num_args);
    int                         yield(int num_rets) { return lua_yield(*this, num_rets); }
    int                         state() const;

public:
    bool                        pending() const;
    void                        pending(bool value);
    void                        wake_up();

public:
    static thread*              get(lua_State& lua_state);
};

void release();

template <typename T>
void bind_class()
{
    luaL_newmetatable(main::get(), T::LUA_METATABLE_NAME.c_str());

    lua_pushvalue(main::get(), -1);
    lua_setfield(main::get(), -2, "__index");
    luaL_setfuncs(main::get(), T::LUA_METHODS, 0);
}

template <typename T, typename B>
void bind_class()
{
    // 새로운 메타테이블 형식을 생성 (T : -1)
    luaL_newmetatable(main::get(), T::LUA_METATABLE_NAME.c_str());
    
    // 상속받을 메타테이블을 가져온다. (B : -1) (T : -2)
    luaL_getmetatable(main::get(), B::LUA_METATABLE_NAME.c_str());

    // 상속받을 메타테이블로 설정한다. (T : -1)
    lua_setmetatable(main::get(), -2);

    lua_pushvalue(main::get(), -1);
    lua_setfield(main::get(), -2, "__index");
    luaL_setfuncs(main::get(), T::LUA_METHODS, 0);
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

    auto ret = static_cast<T*>(lua_touserdata(main::get(), -1));
    lua_remove(main::get(), -1);

    return ret;
}

} } }

#endif // !__LUA_H__
