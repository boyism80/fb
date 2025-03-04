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
#include <functional>
#include <mutex>
#include <format>
#include <thread>
#include <macro.h>
#include <fb/encoding.h>
#include <fb/logger.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <shared_mutex>

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
    }

#define LUA_PENDING (LUA_ERRERR + 1)

namespace fb {

class thread;
class thread_container;

} // namespace fb

namespace fb::lua {

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
 * @brief      This class describes a root.
 */
class root;
/**
 * @brief      This class describes a thread.
 */
class thread;

/**
 * @brief      { function_description }
 *
 * @return     { description_of_the_return_value }
 */
context* new_context();
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
async::task<void> build(const std::string& name, lua_CFunction fn);
/**
 * @brief      { function_description }
 *
 * @param[in]  path  The path
 */
async::task<void> dump(const std::string& path);

/**
 * @brief      This class describes a luable.
 */
class luable
{
public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      { function_description }
     *
     * @param      ctx   The context
     */
    void to_lua(lua_State* ctx) const;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     */
    luable();
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  id       The identifier
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
    int                                                  _state = 0;
    std::shared_ptr<async::task_completion_source<bool>> _promise;
    bool                                                 _auto_release = false;

protected:
    lua_State* _ctx = nullptr;

public:
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
    context& load(const std::string& fmt, Args&&... args);
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
     * @brief      Gets the type.
     *
     * @param[in]  offset  The offset
     *
     * @return     The type.
     */
    std::string get_type(int offset);

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
        if (this->argc() < offset)
            return -1;
        else
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
        if (this->argc() < offset)
            return -1;
        else
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
        if (this->argc() < offset)
            return false;
        else
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
        if (this->argc() < offset)
            return nullptr;
        else
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
    [[nodiscard]] async::task<bool> call(int argc, bool auto_release = true);

    /**
     * @brief      { function_description }
     *
     * @param[in]  argc  The count of arguments
     */
    void resume(int argc);
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
 * @brief      This class describes a root.
 */
class root : public context
{
public:
    using unique_lua_map = std::unordered_map<lua_State*, std::unique_ptr<thread>>;
    using bytecode_set   = std::unordered_map<std::string, std::vector<char>>;

private:
    bytecode_set _bytecodes;
    std::mutex   _mutex;
    fb::thread&  _thread;

public:
    friend class context;

private:

public:
    unique_lua_map idle, busy;

public:
    /**
     * @brief      Constructs a new instance.
     */
    root(fb::thread& thread);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    root(const root&&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~root();

public:
    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    root& operator= (root&) = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    root& operator= (root&&) = delete;

public:
    /**
     * @brief      Loads a file.
     *
     * @param[in]  path  The path
     *
     * @return     { description_of_the_return_value }
     */
    bool dump(const std::string& path);
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
    context* get(lua_State* ctx);
    /**
     * @brief      { function_description }
     *
     * @param      ctx   The context
     *
     * @return     { description_of_the_return_value }
     */
    void release(context& ctx);
    /**
     * @brief      { function_description }
     *
     * @param      ctx   The context
     */
    void revoke(context& ctx);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> switching();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread& initial_thread();

public:
    /**
     * @brief      { function_description }
     *
     * @tparam     T     { description }
     */
    template <typename T>
    void build()
    {
        auto metaname = T::LUA_METATABLE_NAME.c_str();
        luaL_newmetatable(*this, metaname); // [mt]
        lua_pushvalue(*this, -1);           // [mt, mt]
                                            // mt.__index = mt
        lua_setfield(*this, -2, "__index"); // [mt]
                                            // [mt.functions = ...]
        auto metafuncs = T::LUA_METHODS;
        luaL_setfuncs(*this, metafuncs, 0); // []
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
        auto child_metaname = T::LUA_METATABLE_NAME.c_str();
        luaL_newmetatable(*this, child_metaname); // [mt]

        auto parent_metaname = B::LUA_METATABLE_NAME.c_str();
        luaL_getmetatable(*this, parent_metaname); // [mt, bt]
                                                   // mt.__metatable = bt
        lua_setmetatable(*this, -2);               // [mt]
        lua_pushvalue(*this, -1);                  // [mt, mt]
                                                   // mt.__index = mt
        lua_setfield(*this, -2, "__index");        // [mt]
                                                   // mt.functions = ...
        auto child_metafuncs = T::LUA_METHODS;
        luaL_setfuncs(*this, child_metafuncs, 0); // []
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

class context_pool
{
public:
    using base_type  = std::unordered_map<std::thread::id, root*>;
    using setup_func = std::function<void(root& lua)>;

private:
    base_type                                       _roots;
    std::vector<setup_func>                         _setup_funcs;
    std::unordered_map<lua_State*, std::thread::id> _mapping;
    std::shared_mutex                               _mapping_lock;
    fb::thread_container*                           _threads;

public:
    ~context_pool();

public:
    context* pop();
    context* get(lua_State* ctx);
    void     setup(fb::thread_container& threads);
    void     record(lua_State* L);
    void     unrecord(lua_State* L);

    base_type::iterator begin();
    base_type::iterator end();

public:
    static context_pool& ist();
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
 * @brief      { function_description }
 *
 * @tparam     T     { description }
 */
template <typename T>
async::task<void> build()
{
    auto& ist = context_pool::ist();
    for (auto& [_, root] : ist)
    {
        co_await root->switching();
        root->template build<T>();
    }
}

/**
 * @brief      { function_description }
 *
 * @tparam     T     { description }
 * @tparam     B     { description }
 */
template <typename T, typename B>
async::task<void> build()
{
    auto& ist = context_pool::ist();
    for (auto& [_, root] : ist)
    {
        co_await root->switching();
        root->template build<T, B>();
    }
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
async::task<void> env(const char* key, T* data)
{
    auto& ist = context_pool::ist();
    for (auto& [_, root] : ist)
    {
        co_await root->switching();
        root->env(key, data);
    }
}

} // namespace fb::lua

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
fb::lua::context& fb::lua::context::load(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
#if defined DEBUG || defined _DEBUG
    luaL_dofile(*this, fname.c_str());
#else
    auto root = static_cast<fb::lua::root*>(this->owner);
    root->dump(fname);
    if (root->_bytecodes.contains(fname) == false)
    {
        fb::logger::fatal("cannot find script {}", fname);
        return *this;
    }

    auto& bytes = root->_bytecodes[fname];
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
fb::lua::context& fb::lua::context::func(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    lua_getglobal(*this, fname.c_str());
    return *this;
}

#endif // !__LUA_H__
