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
#include <fb/socket.h>
#include <fb/encoding.h>
#include <fb/logger.h>
#include <mmo.h>

#define LUA_PROTOTYPE                                \
    static const struct luaL_Reg LUA_METHODS[];      \
    static const std::string     LUA_METATABLE_NAME; \
    virtual const std::string&   metaname() const    \
    {                                                \
        return this->LUA_METATABLE_NAME;             \
    }

#define IMPLEMENT_LUA_EXTENSION(type, name)                \
    const std::string     type::LUA_METATABLE_NAME = name; \
    const struct luaL_Reg type::LUA_METHODS[]      = {

#define END_LUA_EXTENSION \
    {                     \
        NULL, NULL        \
    }                     \
    }                     \
    ;

#define LUA_PENDING (LUA_ERRERR + 1)

namespace fb { namespace game { namespace lua {

constexpr auto DEFAULT_POOL_SIZE = 1000;

class luable;
class context;
class main;
class thread;

context* get();
context* get(lua_State* ctx);
void     bind_function(const std::string& name, lua_CFunction fn);
void     load(const std::string& path);

class luable
{
public:
    LUA_PROTOTYPE
    void to_lua(lua_State* ctx) const;

protected:
    luable();
    luable(uint32_t id);

public:
    virtual ~luable();

public:
    static int builtin_gc(lua_State* ctx);
};

class context
{
private:
    int _state = 0;

protected:
    lua_State* _ctx = nullptr;

protected:
    context* owner = nullptr;

protected:
    context(lua_State* ctx);
    context(lua_State* ctx, context& owner);
    context(const context&) = delete;
    context(context&&)      = delete;

public:
    virtual ~context() = default;

public:
    context operator= (context&)       = delete;
    context operator= (const context&) = delete;

public:
    template <class... Args>
    context& from(const std::string& fmt, Args&&... args);
    template <class... Args>
    context& func(const std::string& fmt, Args&&... args);

    context& pushstring(const std::string& value);
    context& pushinteger(lua_Integer value);
    context& pushnumber(lua_Number value);
    context& pushnil();
    context& pushboolean(bool value);
    context& pushobject(const luable* object);
    context& pushobject(const luable& object);
    context& push(const void* value);
    context& pop(int offset);

    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    context& pushinteger(T value)
    {
        return this->pushinteger(static_cast<lua_Integer>(value));
    }

    std::string tostring(int offset);
    std::string arg_string(int offset)
    {
        return tostring(offset);
    }
    std::string ret_string(int offset)
    {
        return tostring(-offset);
    }

    int tointeger(int offset)
    {
        return (int)lua_tointeger(*this, offset);
    }
    lua_Integer tonumber(int offset)
    {
        return lua_tonumber(*this, offset);
    }
    int arg_integer(int offset)
    {
        return tointeger(offset);
    }
    int ret_integer(int offset)
    {
        return tointeger(-offset);
    }

    bool toboolean(int offset)
    {
        return lua_toboolean(*this, offset);
    }
    bool arg_boolean(int offset)
    {
        return toboolean(offset);
    }
    bool ret_boolean(int offset)
    {
        return toboolean(-offset);
    }

    template <typename T>
    T* touserdata(int offset)
    {
        return *(T**)lua_touserdata(*this, offset);
    }

    bool is_str(int offset)
    {
        return lua_isstring(*this, offset);
    }
    bool is_obj(int offset)
    {
        return lua_isuserdata(*this, offset);
    }
    bool is_table(int offset)
    {
        return lua_istable(*this, offset);
    }
    bool is_num(int offset)
    {
        return lua_isnumber(*this, offset);
    }
    bool is_nil(int offset)
    {
        return lua_isnil(*this, offset);
    }

    int rawgeti(int offset_t, int offset_e)
    {
        return lua_rawgeti(*this, offset_t, offset_e);
    }
    void rawseti(int offset_t, int offset_e)
    {
        lua_rawseti(*this, offset_t, offset_e);
    }
    int rawlen(int offset_t)
    {
        return (int)lua_rawlen(*this, offset_t);
    }
    void remove(int offset)
    {
        lua_remove(*this, offset);
    }
    void new_table()
    {
        lua_newtable(*this);
    }
    bool next(int offset)
    {
        return lua_next(*this, offset) != 0;
    };

public:
    int  argc();
    bool resume(int argc, bool auto_release = true);
    int  yield(int retc)
    {
        return lua_yield(*this, retc);
    }
    int  state() const;
    void release();
    bool pending() const;
    void pending(bool value);

public:
    operator lua_State* () const;

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
};

class main : public context
{
public:
    using unique_lua_map = std::map<lua_State*, std::unique_ptr<thread>>;
    using bytecode_set   = std::map<std::string, std::vector<char>>;

private:
    bytecode_set _bytecodes;

public:
    friend class context;

private:

public:
    unique_lua_map idle, busy;

public:
    main();
    main(const main&&) = delete;
    ~main();

public:
    main& operator= (main&)  = delete;
    main& operator= (main&&) = delete;

private:

public:
    bool     load_file(const std::string& path);
    context* pop();
    context* get(lua_State& ctx);
    context& release(context& ctx);
    void     revoke(context& ctx);

public:
    template <typename T>
    void bind_class()
    {
        luaL_newmetatable(*this, T::LUA_METATABLE_NAME.c_str()); // [mt]
        lua_pushvalue(*this, -1);                                // [mt, mt]
                                                                 // mt.__index = mt
        lua_setfield(*this, -2, "__index");                      // [mt]
                                                                 // [mt.functions = ...]
        luaL_setfuncs(*this, T::LUA_METHODS, 0);                 // []
    }

    template <typename T, typename B>
    void bind_class()
    {
        luaL_newmetatable(*this, T::LUA_METATABLE_NAME.c_str()); // [mt]
        luaL_getmetatable(*this, B::LUA_METATABLE_NAME.c_str()); // [mt, bt]
                                                                 // mt.__metatable = bt
        lua_setmetatable(*this, -2);                             // [mt]
        lua_pushvalue(*this, -1);                                // [mt, mt]
                                                                 // mt.__index = mt
        lua_setfield(*this, -2, "__index");                      // [mt]
                                                                 // mt.functions = ...
        luaL_setfuncs(*this, T::LUA_METHODS, 0);                 // []
    }

    void bind_function(const std::string& name, lua_CFunction fn)
    {
        lua_register(*this, name.c_str(), fn);
    }
};

class thread : public context
{
public:
    const int ref;

public:
    thread(context& owner);
    thread(const thread&) = delete;
    thread(thread&& ctx);
    ~thread();
};

class container
{
    using main_set   = std::map<uint32_t, std::unique_ptr<main>>;
    using init_func  = std::function<void(main&)>;
    using init_funcs = std::vector<init_func>;

private:
    std::mutex               _mutex;
    main_set                 _mains;
    std::vector<std::string> _scripts;
    init_funcs               _init_funcs;

private:
    container();

public:
    ~container();

public:
    main& get();
    void  init_fn(init_func&& fn);
    void  load(const std::string& path);

public:
    static container& ist();
};

template <typename T>
void bind_class()
{
    auto& ist = container::ist();
    ist.init_fn([](main& m) {
        m.bind_class<T>();
    });
}

template <typename T, typename B>
void bind_class()
{
    auto& ist = container::ist();
    ist.init_fn([](main& m) {
        m.bind_class<T, B>();
    });
}

template <typename T>
void env(const char* key, T* data)
{
    auto& ist = container::ist();
    ist.init_fn([key, data](main& m) {
        m.env(key, data);
    });
}

}}} // namespace fb::game::lua

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
void lua_pushinteger(lua_State* L, T value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

template <typename T>
inline void to_lua(lua_State* ctx, const T* self)
{
    auto allocated = (void**)lua_newuserdata(ctx, sizeof(void**));
    *allocated     = (const void*)self;

    auto metaname  = self->metaname();
    luaL_getmetatable(ctx, metaname.c_str());
    lua_setmetatable(ctx, -2);
}

template <class... Args>
fb::game::lua::context& fb::game::lua::context::from(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
#if defined DEBUG || defined _DEBUG
    luaL_dofile(*this, fname.c_str());
#else
    auto main = static_cast<fb::game::lua::main*>(this->owner);
    if (main->_bytecodes.contains(fname) == false)
    {
        fb::logger::fatal("cannot find script {}", fname);
        return *this;
    }

    auto& bytes = main->_bytecodes[fname];
    if (luaL_loadbuffer(*this, bytes.data(), bytes.size(), 0))
        return *this;

    if (lua_pcall(*this, 0, LUA_MULTRET, 0))
        return *this;
#endif

    return *this;
}

template <class... Args>
fb::game::lua::context& fb::game::lua::context::func(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    lua_getglobal(*this, fname.c_str());
    return *this;
}

#endif // !__LUA_H__
