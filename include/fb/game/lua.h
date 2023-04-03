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
#include <random>
#include <fb/core/socket.h>
#include <fb/core/encoding.h>
#include <fb/game/mmo.h>

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

constexpr auto DEFAULT_POOL_SIZE = 1000;

#pragma region forward declaration
class luable;
class context;
class main;
class thread;
#pragma endregion

#pragma region global function
context*                        get();
context*                        get(lua_State* ctx);
void                            bind_function(const std::string& name, lua_CFunction fn);
#pragma endregion

class luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
    void                        to_lua(lua_State* ctx) const;
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
    static int                  builtin_gc(lua_State* ctx);
#pragma endregion
};

class context
{
#pragma region private field
private:
    int                         _state  = 0;
#pragma endregion

#pragma region protected field
protected:
    lua_State*                  _ctx    = nullptr;
#pragma endregion

#pragma region constructor / destructor
protected:
    context(lua_State* ctx);
    context(const context&)            = delete;
    context(context&&)                 = delete;

public:
    virtual ~context();

public:
    context operator = (context&)       = delete;
    context operator = (const context&) = delete;
#pragma endregion

#pragma region public method
public:
    context&                    from(const char* format, ...);
    context&                    func(const char* format, ...);
                                
    context&                    pushstring(const std::string& value);
    context&                    pushinteger(lua_Integer value);
    context&                    pushnil();
    context&                    pushboolean(bool value);
    context&                    pushobject(const luable* object);
    context&                    pushobject(const luable& object);
    context&                    push(const void* value);

    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    context&                    pushinteger(T value)
    {
        return this->pushinteger(static_cast<lua_Integer>(value));
    }

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
    bool                        resume(int argc);
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
        ::lua_getfield(*this, LUA_REGISTRYINDEX, key);
        auto data = static_cast<T*>(::lua_touserdata(*this, -1));
        lua_pop(*this, 1);

        return data;
    }

    template <typename T>
    void env(const char* key, T* data)
    {
        ::lua_pushlightuserdata(*this, (void*)data);
        ::lua_setfield(*this, LUA_REGISTRYINDEX, key);
    }
#pragma endregion
};

class main : public context
{
#pragma region type definition
public:
    using unique_lua_map        = std::map<lua_State*, std::unique_ptr<thread>>;
    using bytecode_set          = std::map<std::string, std::vector<char>>;
#pragma endregion

#pragma region static field
private:
    static std::mutex           mutex;
    bytecode_set                bytecodes;
#pragma endregion

#pragma region friend
public:
    friend class context;
#pragma endregion

#pragma region private field
private:
    
#pragma endregion

#pragma region public field
public:
    unique_lua_map              idle, busy;
#pragma endregion

#pragma region constructor / destructor
public:
    main();
    main(const main&&)          = delete;
    ~main();

public:
    main& operator = (main&)    = delete;
    main& operator = (main&&)   = delete;
#pragma endregion

#pragma region private method
private:
    
#pragma endregion

#pragma region public method
public:
    bool                        load_file(const std::string& path);
    context*                    pop();
    context*                    get(lua_State& ctx);
    context&                    release(context& ctx);
    void                        revoke(context& ctx);
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
        luaL_newmetatable(*this, T::LUA_METATABLE_NAME.c_str());      // [mt]
        lua_pushvalue(*this, -1);                                     // [mt, mt]
                                                                      // mt.__index = mt
        lua_setfield(*this, -2, "__index");                           // [mt]
                                                                      // [mt.functions = ...]
        luaL_setfuncs(*this, T::LUA_METHODS, 0);                      // []
    }

    template <typename T, typename B>
    void bind_class()
    {
        luaL_newmetatable(*this, T::LUA_METATABLE_NAME.c_str());      // [mt]
        luaL_getmetatable(*this, B::LUA_METATABLE_NAME.c_str());      // [mt, bt]
                                                                      // mt.__metatable = bt
        lua_setmetatable(*this, -2);                                  // [mt]
        lua_pushvalue(*this, -1);                                     // [mt, mt]
                                                                      // mt.__index = mt
        lua_setfield(*this, -2, "__index");                           // [mt]
                                                                      // mt.functions = ...
        luaL_setfuncs(*this, T::LUA_METHODS, 0);                      // []
    }

    void bind_function(const std::string& name, lua_CFunction fn)
    {
        lua_register(*this, name.c_str(), fn);
    }
#pragma endregion
};


class thread : public context
{
public:
    const int               ref;

public:
    thread(lua_State* ctx);
    thread(const thread&) = delete;
    thread(thread&& ctx);
    ~thread();

#pragma region built-in method
public:
    //static int                  builtin_gc(lua_State* ctx);
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

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
void lua_pushinteger(lua_State *L, T value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

template<typename T>
inline void to_lua(lua_State* ctx, const T* self)
{
    auto allocated = (void**)lua_newuserdata(ctx, sizeof(void**));
    *allocated = (const void*)self;

    auto metaname = self->metaname();
    luaL_getmetatable(ctx, metaname.c_str());
    lua_setmetatable(ctx, -2);
}

#endif // !__LUA_H__