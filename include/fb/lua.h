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
#include <fb/locker.h>
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
class thread_switchable;
class async_executor;

} // namespace fb

namespace fb::lua {

constexpr auto DEFAULT_POOL_SIZE = 1000;

template <typename T>
struct is_shared_ptr : std::false_type
{ };

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
{
    using element_type = T;
};

template <typename T>
inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

template <typename T>
struct is_weak_ptr : std::false_type
{ };

template <typename T>
struct is_weak_ptr<std::weak_ptr<T>> : std::true_type
{
    using element_type = T;
};

template <typename T>
constexpr bool is_weak_ptr_v = is_weak_ptr<T>::value;

template <typename T>
struct is_shared_from_this_capable : std::false_type
{ };

template <typename T>
struct is_shared_from_this_capable<std::shared_ptr<T>> : std::true_type
{ };

template <typename T>
struct is_shared_from_this_capable<T*>
{
    using raw_type = std::remove_pointer_t<T>;

    // Check all enable_shared_from_this specializations and thread_switchable inheritance
    template <typename U>
    static constexpr bool is_any_shared_from_this = std::is_base_of_v<std::enable_shared_from_this<U>, raw_type>;

    static constexpr bool value = is_any_shared_from_this<raw_type> ||            // Direct inheritance
                                  is_any_shared_from_this<thread_switchable> ||   // Inherited through thread_switchable
                                  std::is_base_of_v<thread_switchable, raw_type>; // Is a thread_switchable child
};

template <typename T>
struct is_shared_from_this_capable<T&> : is_shared_from_this_capable<T*>
{ };

template <typename T>
inline constexpr bool is_shared_from_this_capable_v = is_shared_from_this_capable<T>::value;

template <typename T>
struct can_shared_from_this
{
    template <typename U>
    static constexpr bool check()
    {
        if constexpr (std::is_base_of_v<std::enable_shared_from_this<U>, U>)
            return true;
        else if constexpr (std::is_base_of_v<thread_switchable, U>)
            return true;
        else
            return false;
    }

    static constexpr bool value = check<T>();
};

template <typename T>
inline constexpr bool can_shared_from_this_v = can_shared_from_this<T>::value;

class luable;
class context;
class root;
class thread;

context* new_context(context* parent = nullptr);

context* get(lua_State* ctx);

async::task<void> build(const std::string& name, lua_CFunction fn);

async::task<void> dump(const std::string& path);

class luable : public std::enable_shared_from_this<luable>
{
public:
    LUA_PROTOTYPE

protected:
    luable();
    luable(uint32_t id);

public:
    virtual ~luable();

public:
    template <typename T>
    static int builtin_gc(lua_State* ctx)
    {
        if constexpr (is_shared_ptr_v<T> || is_weak_ptr_v<T>)
        {
            using element_type = typename T::element_type;
            static_assert(std::is_base_of_v<luable, element_type>, "pointer element type must inherit from luable");

            // Handle weak_ptr stored in userdata
            if constexpr (is_weak_ptr_v<T>)
            {
                auto allocated = static_cast<T*>(lua_touserdata(ctx, 1));
                if (allocated != nullptr)
                    allocated->~T(); // Explicitly call destructor for weak_ptr
            }
            else // shared_ptr case
            {
                auto allocated = static_cast<std::weak_ptr<element_type>*>(lua_touserdata(ctx, 1));
                if (allocated != nullptr)
                    allocated->~weak_ptr<element_type>(); // Explicitly call destructor
            }
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            using pointee_type = std::remove_pointer_t<T>;
            static_assert(std::is_base_of_v<luable, pointee_type>, "Pointer type must point to luable");
        }
        else if constexpr (std::is_base_of_v<luable, T>)
        {
            // For direct luable objects (reference/value types)
        }
        else
        {
            // Skip static_assert for now - inheritance check is complex with multiple inheritance
            // static_assert condition will be checked at runtime instead
        }
        return 0;
    }

    std::weak_ptr<luable> weak_from_this()
    {
        return std::weak_ptr<luable>(this->shared_from_this_as<luable>());
    }

    std::weak_ptr<const luable> weak_from_this() const
    {
        return std::weak_ptr<const luable>(this->shared_from_this_as<luable>());
    }

    template <typename T>
    std::weak_ptr<T> weak_from_this_as()
    {
        static_assert(std::is_base_of_v<luable, T>, "T must inherit from luable");
        return std::static_pointer_cast<T>(this->shared_from_this());
    }

    template <typename T>
    std::weak_ptr<const T> weak_from_this_as() const
    {
        static_assert(std::is_base_of_v<luable, T>, "T must inherit from luable");
        return std::static_pointer_cast<const T>(this->shared_from_this());
    }

    template <typename T>
    std::shared_ptr<T> shared_from_this_as()
    {
        static_assert(std::is_base_of_v<luable, T>, "T must inherit from luable");
        return std::static_pointer_cast<T>(this->shared_from_this());
    }

    template <typename T>
    std::shared_ptr<const T> shared_from_this_as() const
    {
        static_assert(std::is_base_of_v<luable, T>, "T must inherit from luable");
        return std::static_pointer_cast<const T>(this->shared_from_this());
    }
};

class context
{
public:
    using promise_type = std::shared_ptr<async::task_completion_source<bool>>;

private:
    context*     _parent = nullptr;
    int          _state  = 0;
    promise_type _promise;
    bool         _auto_release = false;

protected:
    lua_State*  _ctx = nullptr;
    fb::thread& _initial_thread;

public:
    context* owner = nullptr;

protected:
    context(lua_State* ctx, fb::thread& initial_thread);

    context(lua_State* ctx, context& owner, context* parent = nullptr);

    context(const context&) = delete;

    context(context&&) = delete;

public:
    virtual ~context() = default;

public:
    context operator= (context&) = delete;

    context operator= (const context&) = delete;

public:
    template <class... Args>
    context&                          load(const std::string& fmt, Args&&... args);
    template <class... Args> context& func(const std::string& fmt, Args&&... args);
    context&                          pushstring(const std::string& value);
    context&                          pushinteger(lua_Integer value);
    context&                          pushnumber(lua_Number value);
    context&                          pushnil();
    context&                          pushboolean(bool value);

    template <typename T>
    context& pushobject(const T& value)
    {
        if constexpr (is_shared_ptr_v<T>)
        {
            // Handle shared_ptr<luable>
            using element_type = typename T::element_type;
            static_assert(std::is_base_of_v<luable, element_type>, "shared_ptr element type must inherit from luable");

            if (!value)
            {
                this->pushnil();
                return *this;
            }

            // Store as weak_ptr - preserves exact type
            auto allocated = static_cast<std::weak_ptr<element_type>*>(lua_newuserdata(*this, sizeof(std::weak_ptr<element_type>)));
            new (allocated) std::weak_ptr<element_type>(value);

            auto& metaname = value->metaname();
            luaL_getmetatable(*this, metaname.c_str());
            lua_pushcfunction(*this, luable::builtin_gc<std::weak_ptr<element_type>>);
            lua_setfield(*this, -2, "__gc");
            lua_setmetatable(*this, -2);
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            // Handle luable*
            using element_type = std::remove_cv_t<std::remove_pointer_t<T>>;
            static_assert(std::is_base_of_v<luable, element_type>, "pointer type must point to a type that inherits from luable");

            if (value == nullptr)
            {
                this->pushnil();
                return *this;
            }

            // Try to use shared_from_this if available
            if constexpr (std::is_base_of_v<fb::thread_switchable, element_type>)
            {
                try
                {
                    // Use thread_switchable's shared_from_this_as<T>() - this preserves EXACT type!
                    auto const_shared = value->template shared_from_this_as<element_type>();
                    auto typed_shared = std::const_pointer_cast<element_type>(const_shared);

                    // Store as weak_ptr with the EXACT element_type
                    auto allocated = static_cast<std::weak_ptr<element_type>*>(lua_newuserdata(*this, sizeof(std::weak_ptr<element_type>)));
                    new (allocated) std::weak_ptr<element_type>(typed_shared);

                    auto& metaname = value->metaname();
                    luaL_getmetatable(*this, metaname.c_str());
                    lua_pushcfunction(*this, luable::builtin_gc<std::weak_ptr<element_type>>);
                    lua_setfield(*this, -2, "__gc");
                    lua_setmetatable(*this, -2);
                }
                catch (const std::bad_weak_ptr&)
                {
                    // Fall back to raw pointer
                    auto allocated = static_cast<const element_type**>(lua_newuserdata(*this, sizeof(const element_type*)));
                    *allocated     = value;

                    auto& metaname = value->metaname();
                    luaL_getmetatable(*this, metaname.c_str());
                    lua_pushcfunction(*this, luable::builtin_gc<const element_type*>);
                    lua_setfield(*this, -2, "__gc");
                    lua_setmetatable(*this, -2);
                }
            }
            else
            {
                // Store as raw pointer for non-thread_switchable objects
                auto allocated = static_cast<const element_type**>(lua_newuserdata(*this, sizeof(const element_type*)));
                *allocated     = value;

                auto& metaname = value->metaname();
                luaL_getmetatable(*this, metaname.c_str());
                lua_pushcfunction(*this, luable::builtin_gc<const element_type*>);
                lua_setfield(*this, -2, "__gc");
                lua_setmetatable(*this, -2);
            }
        }
        else
        {
            // Handle luable&
            using element_type = std::remove_cv_t<std::remove_reference_t<T>>;
            static_assert(std::is_base_of_v<luable, element_type>, "reference type must refer to a type that inherits from luable");

            // Try to use shared_from_this if available
            if constexpr (std::is_base_of_v<fb::thread_switchable, element_type>)
            {
                try
                {
                    // Use thread_switchable's shared_from_this_as<T>() - this preserves EXACT type!
                    auto const_shared = value.template shared_from_this_as<element_type>();
                    auto typed_shared = std::const_pointer_cast<element_type>(const_shared);

                    // Store as weak_ptr with the EXACT element_type
                    auto allocated = static_cast<std::weak_ptr<element_type>*>(lua_newuserdata(*this, sizeof(std::weak_ptr<element_type>)));
                    new (allocated) std::weak_ptr<element_type>(typed_shared);

                    auto& metaname = value.metaname();
                    luaL_getmetatable(*this, metaname.c_str());
                    lua_pushcfunction(*this, luable::builtin_gc<std::weak_ptr<element_type>>);
                    lua_setfield(*this, -2, "__gc");
                    lua_setmetatable(*this, -2);
                }
                catch (const std::bad_weak_ptr&)
                {
                    // Fall back to raw pointer
                    auto allocated = static_cast<const element_type**>(lua_newuserdata(*this, sizeof(const element_type*)));
                    *allocated     = &value;

                    auto& metaname = value.metaname();
                    luaL_getmetatable(*this, metaname.c_str());
                    lua_pushcfunction(*this, luable::builtin_gc<const element_type*>);
                    lua_setfield(*this, -2, "__gc");
                    lua_setmetatable(*this, -2);
                }
            }
            else
            {
                // Store as raw pointer for non-thread_switchable objects
                auto allocated = static_cast<const element_type**>(lua_newuserdata(*this, sizeof(const element_type*)));
                *allocated     = &value;

                auto& metaname = value.metaname();
                luaL_getmetatable(*this, metaname.c_str());
                lua_pushcfunction(*this, luable::builtin_gc<const element_type*>);
                lua_setfield(*this, -2, "__gc");
                lua_setmetatable(*this, -2);
            }
        }

        return *this;
    }
    context& push(const void* value);
    context& pop(int offset);

    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    context& pushinteger(T value)
    {
        return this->pushinteger(static_cast<lua_Integer>(value));
    }

    std::string       get_type(int offset);
    std::string       metatable(int offset);
    std::string       basetable(const std::string& metaname);
    std::string       tostring(int offset, const std::string& default_value = "");
    std::string       arg_string(int offset);
    std::string       ret_string(int offset);
    async::task<void> switching();
    int               tointeger(int offset, int default_value = 0);
    lua_Integer       tonumber(int offset, lua_Integer default_value = 0);

    template <typename T>
    T toenum(int offset, T default_value = (T)0)
    {
        if (this->argc() < offset)
            return default_value;
        else if (lua_type(*this, offset) != LUA_TNUMBER)
            return default_value;
        else
            return static_cast<T>(lua_tointeger(*this, offset));
    }
    int  arg_integer(int offset);
    int  ret_integer(int offset);
    bool toboolean(int offset, bool default_value = false);
    bool arg_boolean(int offset);
    bool ret_boolean(int offset);

    template <typename T>
    bool is_userdata(int offset)
    {
        if (this->is_obj(offset) == false)
            return false;

        // T is always a luable type (not pointer)
        auto metaname = this->metatable(offset);
        while (metaname.empty() == false)
        {
            if (metaname == T::LUA_METATABLE_NAME)
                return true;

            metaname = this->basetable(metaname);
        }
        return false;
    }

    template <typename T>
    auto touserdata(int offset)
    {
        // T is always a luable type (not pointer)
        static_assert(std::is_base_of_v<luable, T>, "T must inherit from luable");

        // Return nullptr for invalid cases
        if (this->argc() < offset)
            return std::conditional_t<can_shared_from_this_v<T>, std::shared_ptr<T>, T*>{nullptr};
        else if (lua_type(*this, offset) != LUA_TUSERDATA)
            return std::conditional_t<can_shared_from_this_v<T>, std::shared_ptr<T>, T*>{nullptr};
        else if (this->is_userdata<T>(offset) == false)
            return std::conditional_t<can_shared_from_this_v<T>, std::shared_ptr<T>, T*>{nullptr};

        // Case 1: Type that can use shared_from_this -> return shared_ptr<T>
        if constexpr (can_shared_from_this_v<T>)
        {
            auto weak = static_cast<std::weak_ptr<T>*>(lua_touserdata(*this, offset));
            if (auto shared = weak->lock())
                return shared;
            return std::shared_ptr<T>{nullptr};
        }
        // Case 2: Regular luable type -> return T*
        else
        {
            auto ptr = static_cast<T* const*>(lua_touserdata(*this, offset));
            return *ptr;
        }
    }

    bool is_string(int offset);
    bool is_obj(int offset);
    bool is_function(int offset);
    bool is_table(int offset);
    bool is_number(int offset);
    bool is_nil(int offset);
    int  rawgeti(int offset_t, int offset_e);
    void rawseti(int offset_t, int offset_e);
    int  rawlen(int offset_t);
    void remove(int offset);
    void new_table();
    bool next(int offset);

public:
    int                             argc();
    [[nodiscard]] async::task<bool> call(int argc, bool auto_release = true, int* n = nullptr);
    void                            resume(int argc, int* n = nullptr);
    int                             yield(int retc);
    int                             state() const;
    void                            release();
    bool                            pending() const;
    void                            pending(bool value);
    void                            parent(context* parent);
    context*                        parent() const;
    int                             ensure_yield(fb::async_executor& executor, std::weak_ptr<fb::thread_switchable> weak, std::function<int(bool)> fn, bool no_yield = false);
    int                             ensure_resume(fb::async_executor& executor, std::weak_ptr<fb::thread_switchable> weak, std::function<int()> fn, bool force_resume = false);

public:
    operator lua_State* () const;

public:
    template <typename T> T* env(const char* key)
    {
        ::lua_getfield(*this, LUA_REGISTRYINDEX, key);
        auto data = static_cast<T*>(::lua_touserdata(*this, -1));
        lua_pop(*this, 1);

        return data;
    }

    template <typename T> void env(const char* key, T* data)
    {
        ::lua_pushlightuserdata(*this, (void*)data);
        ::lua_setfield(*this, LUA_REGISTRYINDEX, key);
    }
};

class root : public context
{
public:
    using unique_lua_map = std::unordered_map<lua_State*, std::unique_ptr<thread>>;
    using bytecode_set   = std::unordered_map<std::string, std::vector<char>>;

private:
    bytecode_set _bytecodes;

public:
    friend class context;

private:

public:
    unique_lua_map idle, busy;

public:
    root(fb::thread& thread);
    root(const root&&) = delete;
    ~root();

public:
    root& operator= (root&)  = delete;
    root& operator= (root&&) = delete;

public:
    bool        dump(const std::string& path);
    context*    pop(context* parent);
    context*    get(lua_State* ctx);
    void        release(context& ctx);
    void        revoke(context& ctx);
    fb::thread& initial_thread();

public:
    template <typename T> void build()
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

    template <typename T, typename B> void build()
    {
        auto child_metaname = T::LUA_METATABLE_NAME.c_str();
        luaL_newmetatable(*this, child_metaname); // [mt]

        auto parent_metaname = B::LUA_METATABLE_NAME.c_str();
        luaL_getmetatable(*this, parent_metaname); // [mt, bt]
        lua_setmetatable(*this, -2);               // [mt]
        luaL_getmetatable(*this, parent_metaname); // [mt, bt]
        lua_setfield(*this, -2, "__parent");       // [mt] mt.__metatable = bt
        lua_pushvalue(*this, -1);                  // [mt, mt]
                                                   // mt.__index = mt
        lua_setfield(*this, -2, "__index");        // [mt]
                                                   // mt.functions = ...
        auto child_metafuncs = T::LUA_METHODS;
        luaL_setfuncs(*this, child_metafuncs, 0); // []
    }

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
    base_type               _roots;
    std::vector<setup_func> _setup_funcs;
    fb::thread_container*   _threads;

public:
    ~context_pool();

public:
    context* pop(context* parent);
    context* get(lua_State* ctx);
    void     setup(fb::thread_container& threads);

    base_type::iterator begin();
    base_type::iterator end();

public:
    static context_pool& ist();
};

class thread : public context
{
public:
    const int ref;

public:
    thread(context& owner, context* parent);
    thread(const thread& other) = delete;
    thread(thread&& ctx);
    ~thread();
};

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

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
void lua_pushinteger(lua_State* L, T value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

template <class... Args>
fb::lua::context& fb::lua::context::load(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
#if defined DEBUG || defined _DEBUG
    if (luaL_dofile(*this, fname.c_str()) != LUA_OK)
    {
        auto error = lua_tostring(*this, -1);
        this->pop(1); // pop error message
        throw std::runtime_error(error);
    }
#else
    auto root = static_cast<fb::lua::root*>(this->owner);
    root->dump(fname);

    auto it = root->_bytecodes.find(fname);
    if (it == root->_bytecodes.end())
        throw std::runtime_error(std::format("cannot find script {}", fname));

    const auto& bytes = it->second;
    if (luaL_loadbuffer(*this, bytes.data(), bytes.size(), 0))
        throw std::runtime_error(std::format("cannot load script {}", fname));

    if (lua_pcall(*this, 0, LUA_MULTRET, 0))
        throw std::runtime_error(std::format("cannot run script {}", fname));

#endif
    return *this;
}

template <class... Args>
fb::lua::context& fb::lua::context::func(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    lua_getglobal(*this, fname.c_str());
    return *this;
}

#endif // !__LUA_H__
