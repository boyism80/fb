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

#pragma region definition
#define LUA_PROTOTYPE                       static const struct luaL_Reg    LUA_METHODS[];\
                                            static const std::string        LUA_METATABLE_NAME;\
                                            virtual const std::string& metaname() const { return this->LUA_METATABLE_NAME; }

#define IMPLEMENT_LUA_EXTENSION(type, name) const std::string type::LUA_METATABLE_NAME = name;\
                                            const struct luaL_Reg type::LUA_METHODS[] = \
                                            {

#define END_LUA_EXTENSION                   {NULL, NULL}\
                                            };

#define LUA_PENDING (LUA_ERRERR+1)
#pragma endregion

namespace fb { namespace game { namespace lua {

constexpr auto DEFAULT_POOL_SIZE = 100;

#pragma region forward declaration
class luable;
class lua;
class main;
#pragma endregion

#pragma region global function
lua&                            get();
lua*                            get(lua_State* lua);
void                            reserve(int capacity = DEFAULT_POOL_SIZE);
void                            bind_function(const std::string& name, lua_CFunction fn);
#pragma endregion

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

#pragma region built-in method
public:
    static int                  builtin_gc(lua_State* lua);
#pragma endregion
};

class lua
{
#pragma region friend
private:
    friend class main;
#pragma endregion

#pragma region static field
private:
    static std::mutex           _mutex;
#pragma endregion

#pragma region private field
private:
    int                         _state  = 0;
    int                         _ref    = 0;
#pragma endregion

#pragma region protected field
protected:
    lua_State*                  _lua    = nullptr;
#pragma endregion

#pragma region constructor / destructor
protected:
    lua(lua_State* lua);
    lua(const lua&) = delete;
    lua(lua&&) = delete;

public:
    virtual ~lua();

public:
    lua& operator = (lua&) = delete;
    lua& operator = (const lua&) = delete;
#pragma endregion

#pragma region private method
private:
    void                        bind_builtin_functions();
#pragma endregion

#pragma region public method
public:
    lua&                        from(const char* format, ...);
    lua&                        func(const char* format, ...);
                                
    lua&                        pushstring(const std::string& value);
    lua&                        pushinteger(lua_Integer value);
    lua&                        pushnil();
    lua&                        pushboolean(bool value);
    lua&                        pushobject(const luable* object);
    lua&                        pushobject(const luable& object);

    const std::string           tostring(int offset);
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
    int                         rawlen(int offset_t) { return (int)lua_rawlen(*this, offset_t); }
    void                        remove(int offset) { lua_remove(*this, offset); }
    void                        new_table() { lua_newtable(*this); }

public:
    int                         argc() const;

public:
    lua&                        resume(int argc);
    int                         yield(int retc) { return lua_yield(*this, retc); }
    int                         state() const;
    void                        release();

public:
    bool                        pending() const;
    void                        pending(bool value);
#pragma endregion

#pragma region operator
public:
    operator                    lua_State* () const;
#pragma endregion

#pragma region template method
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
#pragma endregion
};


class main
{
#pragma region friend
public:
    friend class lua;
#pragma endregion

#pragma region type definition
public:
    using builtin_func_map      = std::map<std::string, const luaL_Reg*>;
    using builtin_funcs         = std::map<std::string, lua_CFunction>;
    using relation_map          = std::map<std::string, std::string>;
    using environment_map       = std::map<std::string, void*>;
    using unique_lua_map        = std::map<lua_State*, std::unique_ptr<lua>>;
#pragma endregion

#pragma region private field
private:
    builtin_func_map            builtin_local_funcs;
    builtin_funcs               builtin_global_funcs;
    relation_map                relations;
    environment_map             environments;
    std::list<std::string>      inheritances;
#pragma endregion

#pragma region public field
public:
    unique_lua_map              idle, busy;
#pragma endregion

#pragma region static field
private:
    static std::unique_ptr<main> _instance;
#pragma endregion

#pragma region constructor / destructor
public:
    main() = default;
    main(const main&&) = delete;
    ~main() = default;

public:
    main& operator = (main&) = delete;
    main& operator = (main&&) = delete;
#pragma endregion

#pragma region private method
private:
    void                        update_inheritances();
#pragma endregion

#pragma region public method
public:
    void                        reserve(int capacity = DEFAULT_POOL_SIZE);
    lua&                        alloc();
    lua*                        get(lua_State& lua);
    lua&                        release(lua& lua);
#pragma endregion

#pragma region static method
public:
    static main&                get();
#pragma endregion

#pragma region template method
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
#pragma endregion
};

#pragma region global template function
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
#pragma endregion

} } }

#endif // !__LUA_H__
