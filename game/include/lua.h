#ifndef __LUA_H__
#define __LUA_H__

extern "C"
{
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

#include <vector>
#include <string>
#include <map>
#include <list>
#include <random>
#include <fb/socket.h>
#include <fb/encoding.h>
#include <fb/logger.h>

#define LUA_PROTOTYPE                                \
    static const struct luaL_Reg LUA_METHODS[];      \
    static const std::string     LUA_METATABLE_NAME; \
    virtual const std::string&   metaname() const    \
    {                                                \
        return this->LUA_METATABLE_NAME;             \
    }

/**
 * @brief      { function_description }
 *
 * @param      type  The type
 * @param      name  The name
 *
 * @return     { description_of_the_return_value }
 */
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

/**
 * @brief      This class describes a luable.
 */
class luable;
/**
 * @brief      This class describes a context.
 */
class context;
/**
 * @brief      This class describes a main.
 */
class main;
/**
 * @brief      This class describes a thread.
 */
class thread;

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
context* get();
/**
 * @brief      Gets the specified context.
 *
 * @param      ctx   The context
 *
 * @return     { description_of_the_return_value }
 */
context* get(lua_State* ctx);
/**
 * @brief      { function_description }
 *
 * @param[in]  name  The name
 * @param[in]  fn    The function
 */
void build(const std::string& name, lua_CFunction fn);
/**
 * @brief      { function_description }
 *
 * @param[in]  path  The path
 */
void load(const std::string& path);

/**
 * @brief      This class describes a luable.
 */
class luable
{
public:
    /**
     * @brief      { function_description }
     *
     * @param      ctx   The context
     */
    LUA_PROTOTYPE
    void to_lua(lua_State* ctx) const;

protected:
    /**
     * @brief      Constructs a new instance.
     */
    luable();
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  id    The identifier
     */
    luable(uint32_t id);

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~luable();

public:
    /**
     * @brief      { function_description }
     *
     * @param      ctx   The context
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_gc(lua_State* ctx);
};

/**
 * @brief      This class describes a context.
 */
class context
{
private:
    int _state = 0;

protected:
    lua_State* _ctx = nullptr;

protected:
    context* owner = nullptr;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      ctx   The context
     */
    context(lua_State* ctx);
    /**
     * @brief      Constructs a new instance.
     *
     * @param      ctx    The context
     * @param      owner  The owner
     */
    context(lua_State* ctx, context& owner);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    context(const context&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      <unnamed>  { parameter_description }
     */
    context(context&&) = delete;

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~context() = default;

public:
    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    context operator= (context&) = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    context operator= (const context&) = delete;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    context& from(const std::string& fmt, Args&&... args);
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    context& func(const std::string& fmt, Args&&... args);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    context& pushstring(const std::string& value);
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    context& pushinteger(lua_Integer value);
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    context& pushnumber(lua_Number value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    context& pushnil();
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    context& pushboolean(bool value);
    /**
     * @brief      { function_description }
     *
     * @param[in]  object  The object
     *
     * @return     { description_of_the_return_value }
     */
    context& pushobject(const luable* object);
    /**
     * @brief      { function_description }
     *
     * @param[in]  object  The object
     *
     * @return     { description_of_the_return_value }
     */
    context& pushobject(const luable& object);
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    context& push(const void* value);
    /**
     * @brief      Pops the given offset.
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    context& pop(int offset);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value      The value
     *
     * @tparam     T          { description }
     * @tparam     <unnamed>  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    context& pushinteger(T value)
    {
        return this->pushinteger(static_cast<lua_Integer>(value));
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    std::string tostring(int offset);
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    std::string arg_string(int offset)
    {
        return tostring(offset);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    std::string ret_string(int offset)
    {
        return tostring(-offset);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    int tointeger(int offset)
    {
        return (int)lua_tointeger(*this, offset);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     The lua integer.
     */
    lua_Integer tonumber(int offset)
    {
        return lua_tonumber(*this, offset);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    int arg_integer(int offset)
    {
        return tointeger(offset);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    int ret_integer(int offset)
    {
        return tointeger(-offset);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    bool toboolean(int offset)
    {
        return lua_toboolean(*this, offset);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    bool arg_boolean(int offset)
    {
        return toboolean(offset);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    bool ret_boolean(int offset)
    {
        return toboolean(-offset);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @tparam     T       { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename T>
    T* touserdata(int offset)
    {
        return *(T**)lua_touserdata(*this, offset);
    }

    /**
     * @brief      Determines whether the specified offset is string.
     *
     * @param[in]  offset  The offset
     *
     * @return     True if the specified offset is string, False otherwise.
     */
    bool is_str(int offset)
    {
        return lua_isstring(*this, offset);
    }
    /**
     * @brief      Determines whether the specified offset is object.
     *
     * @param[in]  offset  The offset
     *
     * @return     True if the specified offset is object, False otherwise.
     */
    bool is_obj(int offset)
    {
        return lua_isuserdata(*this, offset);
    }
    /**
     * @brief      Determines whether the specified offset is table.
     *
     * @param[in]  offset  The offset
     *
     * @return     True if the specified offset is table, False otherwise.
     */
    bool is_table(int offset)
    {
        return lua_istable(*this, offset);
    }
    /**
     * @brief      Determines whether the specified offset is number.
     *
     * @param[in]  offset  The offset
     *
     * @return     True if the specified offset is number, False otherwise.
     */
    bool is_num(int offset)
    {
        return lua_isnumber(*this, offset);
    }
    /**
     * @brief      Determines whether the specified offset is nil.
     *
     * @param[in]  offset  The offset
     *
     * @return     True if the specified offset is nil, False otherwise.
     */
    bool is_nil(int offset)
    {
        return lua_isnil(*this, offset);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  offset_t  The offset
     * @param[in]  offset_e  The offset e
     *
     * @return     { description_of_the_return_value }
     */
    int rawgeti(int offset_t, int offset_e)
    {
        return lua_rawgeti(*this, offset_t, offset_e);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset_t  The offset
     * @param[in]  offset_e  The offset e
     */
    void rawseti(int offset_t, int offset_e)
    {
        lua_rawseti(*this, offset_t, offset_e);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset_t  The offset
     *
     * @return     { description_of_the_return_value }
     */
    int rawlen(int offset_t)
    {
        return (int)lua_rawlen(*this, offset_t);
    }
    /**
     * @brief      Removes the specified offset.
     *
     * @param[in]  offset  The offset
     */
    void remove(int offset)
    {
        lua_remove(*this, offset);
    }
    /**
     * @brief      { function_description }
     */
    void new_table()
    {
        lua_newtable(*this);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  offset  The offset
     *
     * @return     { description_of_the_return_value }
     */
    bool next(int offset)
    {
        return lua_next(*this, offset) != 0;
    };

public:
    /**
     * @brief      Counts the number of .
     *
     * @return     Number of .
     */
    int argc();
    /**
     * @brief      { function_description }
     *
     * @param[in]  argc          The count of arguments
     * @param[in]  auto_release  The automatic release
     *
     * @return     { description_of_the_return_value }
     */
    bool resume(int argc, bool auto_release = true);
    /**
     * @brief      { function_description }
     *
     * @param[in]  retc  The retc
     *
     * @return     { description_of_the_return_value }
     */
    int yield(int retc)
    {
        return lua_yield(*this, retc);
    }
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    int state() const;
    /**
     * @brief      { function_description }
     */
    void release();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool pending() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void pending(bool value);

public:
    operator lua_State* () const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  key   The key
     *
     * @tparam     T     { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename T>
    T* env(const char* key)
    {
        ::lua_getfield(*this, LUA_REGISTRYINDEX, key);
        auto data = static_cast<T*>(::lua_touserdata(*this, -1));
        lua_pop(*this, 1);

        return data;
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  key   The key
     * @param      data  The data
     *
     * @tparam     T     { description }
     */
    template <typename T>
    void env(const char* key, T* data)
    {
        ::lua_pushlightuserdata(*this, (void*)data);
        ::lua_setfield(*this, LUA_REGISTRYINDEX, key);
    }
};

/**
 * @brief      This class describes a main.
 */
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
    /**
     * @brief      Constructs a new instance.
     */
    main();
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    main(const main&&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~main();

public:
    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    main& operator= (main&) = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    main& operator= (main&&) = delete;

private:

public:
    /**
     * @brief      Loads a file.
     *
     * @param[in]  path  The path
     *
     * @return     { description_of_the_return_value }
     */
    bool load_file(const std::string& path);
    /**
     * @brief      Pops the object.
     *
     * @return     { description_of_the_return_value }
     */
    context* pop();
    /**
     * @brief      Gets the specified context.
     *
     * @param      ctx   The context
     *
     * @return     { description_of_the_return_value }
     */
    context* get(lua_State& ctx);
    /**
     * @brief      { function_description }
     *
     * @param      ctx   The context
     *
     * @return     { description_of_the_return_value }
     */
    context& release(context& ctx);
    /**
     * @brief      { function_description }
     *
     * @param      ctx   The context
     */
    void revoke(context& ctx);

public:
    /**
     * @brief      { function_description }
     *
     * @tparam     T     { description }
     */
    template <typename T>
    void build()
    {
        luaL_newmetatable(*this, T::LUA_METATABLE_NAME.c_str()); // [mt]
        lua_pushvalue(*this, -1);                                // [mt, mt]
                                                                 // mt.__index = mt
        lua_setfield(*this, -2, "__index");                      // [mt]
                                                                 // [mt.functions = ...]
        luaL_setfuncs(*this, T::LUA_METHODS, 0);                 // []
    }

    /**
     * @brief      { function_description }
     *
     * @tparam     T     { description }
     * @tparam     B     { description }
     */
    template <typename T, typename B>
    void build()
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

    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     * @param[in]  fn    The function
     */
    void build(const std::string& name, lua_CFunction fn)
    {
        lua_register(*this, name.c_str(), fn);
    }
};

/**
 * @brief      This class describes a thread.
 */
class thread : public context
{
public:
    const int ref;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The owner
     */
    thread(context& owner);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    thread(const thread&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      ctx   The context
     */
    thread(thread&& ctx);
    /**
     * @brief      Destroys the object.
     */
    ~thread();
};

/**
 * @brief      This class describes a container.
 */
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
    /**
     * @brief      Constructs a new instance.
     */
    container();

public:
    /**
     * @brief      Destroys the object.
     */
    ~container();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    main& get();
    /**
     * @brief      Initializes the function.
     *
     * @param      fn    The function
     */
    void init_fn(init_func&& fn);
    /**
     * @brief      { function_description }
     *
     * @param[in]  path  The path
     */
    void load(const std::string& path);

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    static container& ist();
};

/**
 * @brief      { function_description }
 *
 * @tparam     T     { description }
 */
template <typename T>
void build()
{
    auto& ist = container::ist();
    ist.init_fn([](main& m) {
        m.build<T>();
    });
}

/**
 * @brief      { function_description }
 *
 * @tparam     T     { description }
 * @tparam     B     { description }
 */
template <typename T, typename B>
void build()
{
    auto& ist = container::ist();
    ist.init_fn([](main& m) {
        m.build<T, B>();
    });
}

/**
 * @brief      { function_description }
 *
 * @param[in]  key   The key
 * @param      data  The data
 *
 * @tparam     T     { description }
 */
template <typename T>
void env(const char* key, T* data)
{
    auto& ist = container::ist();
    ist.init_fn([key, data](main& m) {
        m.env(key, data);
    });
}

}}} // namespace fb::game::lua

/**
 * @brief      { function_description }
 *
 * @param      L          { parameter_description }
 * @param[in]  value      The value
 *
 * @tparam     T          { description }
 * @tparam     <unnamed>  { description }
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
void lua_pushinteger(lua_State* L, T value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

/**
 * @brief      { function_description }
 *
 * @param      ctx   The context
 * @param[in]  self  The object
 *
 * @tparam     T     { description }
 */
template <typename T>
inline void to_lua(lua_State* ctx, const T* self)
{
    auto allocated = (void**)lua_newuserdata(ctx, sizeof(void**));
    *allocated     = (const void*)self;

    auto metaname = self->metaname();
    luaL_getmetatable(ctx, metaname.c_str());
    lua_setmetatable(ctx, -2);
}

/**
 * @brief      { function_description }
 *
 * @param[in]  fmt   The format
 * @param      args  The arguments
 *
 * @tparam     Args  { description }
 *
 * @return     { description_of_the_return_value }
 */
template <class... Args>
fb::game::lua::context& fb::game::lua::context::from(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
#if defined DEBUG || defined _DEBUG
    luaL_dofile(*this, fname.c_str());
#else
    auto main = static_cast<fb::game::lua::main*>(this->owner);
    main->load_file(fname);
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

/**
 * @brief      { function_description }
 *
 * @param[in]  fmt   The format
 * @param      args  The arguments
 *
 * @tparam     Args  { description }
 *
 * @return     { description_of_the_return_value }
 */
template <class... Args>
fb::game::lua::context& fb::game::lua::context::func(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    lua_getglobal(*this, fname.c_str());
    return *this;
}

#endif // !__LUA_H__
