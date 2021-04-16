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
#include <map>
#include <list>
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

constexpr auto DEFAULT_POOL_SIZE = 100;

class luable;
class lua;
class main;

lua&                            get();
lua*                            get(lua_State* lua);
void                            reserve(int capacity = DEFAULT_POOL_SIZE);
void                            release();
void                            bind_function(const std::string& name, lua_CFunction fn);

class luable
{
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

class lua
{
private:
    static std::mutex           _mutex;

private:
    friend class main;

private:
    int                         _state;
    int                         _ref;

protected:
    lua_State*                  _lua;

protected:
    lua(lua_State* lua);
    lua(const lua&) = delete;

public:
    virtual ~lua();

private:
    void                        bind_builtin_functions();

public:
    lua&                        from(const char* format, ...);
    lua&                        func(const char* format, ...);
                                
    lua&                        pushstring(const std::string& value);
    lua&                        pushinteger(lua_Integer value);
    lua&                        pushnil();
    lua&                        pushboolean(bool value);
    lua&                        pushobject(const luable* object);
    lua&                        pushobject(const luable& object);

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
    int                         argc() const;

public:
    int                         resume(int argc);
    int                         yield(int num_rets) { return lua_yield(*this, num_rets); }
    int                         state() const;
    void                        release();

public:
    bool                        pending() const;
    void                        pending(bool value);


public:
    operator                    lua_State* () const;

public:
    template <typename T>
    T* env(const char* key)
    {
        lua_pushstring(*this, key);
        lua_gettable(*this, LUA_REGISTRYINDEX);

        auto ret = static_cast<T*>(lua_touserdata(*this, -1));
        lua_remove(*this, -1);

        return ret;
    }
};


class main
{
public:
    friend class lua;

public:
    typedef std::map<std::string, const luaL_Reg*>  builtin_func_map;
    typedef std::map<std::string, lua_CFunction>    builtin_funcs;
    typedef std::map<std::string, std::string>      relation_map;
    typedef std::map<std::string, void*>            environment_map;

private:
    builtin_func_map            builtin_local_funcs;
    builtin_funcs               builtin_global_funcs;
    relation_map                relations;
    environment_map             environments;
    std::list<std::string>      inheritances;

public:
    std::map<lua_State*, lua*>       idle, busy;

private:
    static main* _instance;

public:
    main();
    ~main();

private:
    void                        update_inheritances();
    void                        clear();

public:
    void                        reserve(int capacity = DEFAULT_POOL_SIZE);
    lua&                        alloc();
    lua*                        get(lua_State& lua);
    void                        release(lua& lua);

public:
    static main&                get();
    static void                 release();

public:
    template <typename T>
    void bind_class()
    {
        auto name = T::LUA_METATABLE_NAME.c_str();
        auto methods = T::LUA_METHODS;

        this->builtin_local_funcs.insert(std::make_pair(name, methods));
        this->update_inheritances();
    }

    template <typename T, typename B>
    void bind_class()
    {
        auto derived = T::LUA_METATABLE_NAME.c_str();
        auto base = B::LUA_METATABLE_NAME.c_str();
        this->relations.insert(std::make_pair(derived, base));

        auto methods = T::LUA_METHODS;
        this->builtin_local_funcs.insert(std::make_pair(derived, methods));
        this->update_inheritances();
    }

    void bind_function(const std::string& name, lua_CFunction func)
    {
        this->builtin_global_funcs.insert(std::make_pair(name, func));
    }

    template <typename T>
    void env(const char* key, T* data)
    {
        this->environments.insert(std::make_pair(std::string(key), (void*)data));
    }
};

template <typename T>
void bind_class()
{
    auto& main = fb::game::lua::main::get();
    main.bind_class<T>();
}

template <typename T, typename B>
void bind_class()
{
    auto& main = fb::game::lua::main::get();
    main.bind_class<T, B>();
}

template <typename T>
void env(const char* key, T* data)
{
    auto& main = fb::game::lua::main::get();
    main.env(key, data);
}

} } }

#endif // !__LUA_H__
