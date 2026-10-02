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
#include <string_view>
#include <map>
#include <list>
#include <cstdint>
#include <unordered_set>
#include <random>
#include <functional>
#include <mutex>
#include <format>
#include <thread>
#include <macro.h>
#include <fb/encoding.h>
#include <fb/logger.h>
#include <fb/synchronized.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_then.h>
#include <shared_mutex>
#include <optional>
#include <json/json.h>

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

namespace fb {

class thread;
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

struct call_options
{
    bool auto_resume_parent = true;
};

context* get(lua_State* ctx);

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
    // An object userdata holds either std::weak_ptr<luable> (shared-owned) or luable* (not shared-owned);
    // the two are told apart by the userdata size.
    static bool is_weak_userdata(lua_State* ctx, int offset)
    {
        static_assert(sizeof(std::weak_ptr<luable>) != sizeof(luable*));
        return lua_rawlen(ctx, offset) == sizeof(std::weak_ptr<luable>);
    }

    static int builtin_gc(lua_State* ctx)
    {
        if (is_weak_userdata(ctx, 1))
            static_cast<std::weak_ptr<luable>*>(lua_touserdata(ctx, 1))->~weak_ptr();
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
    class guard
    {
    private:
        context* _ctx = nullptr;

    public:
        guard() = default;
        explicit guard(context* ctx);
        ~guard();

        guard(guard&& other) noexcept;
        guard& operator= (guard&& other) noexcept;
        guard(const guard&)             = delete;
        guard& operator= (const guard&) = delete;

    public:
        context* get() const
        {
            return this->_ctx;
        }

        context* operator->() const
        {
            return this->_ctx;
        }

        context& operator* () const
        {
            return *this->_ctx;
        }

        explicit operator bool () const
        {
            return this->_ctx != nullptr;
        }
    };

public:
    using promise_type = std::shared_ptr<async::task_completion_source<bool>>;

private:
    context*     _parent = nullptr;
    promise_type _promise;
    call_options _options;
    context**    _dialog_slot     = nullptr; // &character::dialog while parked
    int          _running         = 0;
    bool         _release_pending = false;
    std::string  _script_path;

    void finish_resume();

protected:
    lua_State*  _ctx = nullptr;
    fb::thread& _initial_thread;

public:
    fb::async_executor& executor;
    context*            owner = nullptr;
    int                 ref   = LUA_NOREF;

protected:
    context(fb::async_executor& executor, lua_State* ctx, fb::thread& initial_thread);
    context(fb::async_executor& executor, lua_State* ctx, context& owner, context* parent = nullptr);
    context(const context&) = delete;
    context(context&&)      = delete;

public:
    virtual ~context() = default;

public:
    context operator= (context&)       = delete;
    context operator= (const context&) = delete;

public:
    template <class... Args> bool load(std::string_view fmt, Args&&... args);
    template <class... Args> bool execute(std::string_view fmt, Args&&... args);
    template <class... Args> bool dofile(std::string_view fmt, Args&&... args);
    template <class... Args> bool func(std::string_view fmt, Args&&... args);
    context&                      pushstring(std::string_view value);
    context&                      pushinteger(lua_Integer value);
    context&                      pushnumber(lua_Number value);
    context&                      pushnil();
    context&                      pushboolean(bool value);
    context&                      pushjson(const Json::Value& json);

    template <typename T>
    context& pushobject(const T& value)
    {
        if constexpr (is_shared_ptr_v<T>)
        {
            static_assert(std::is_base_of_v<luable, typename T::element_type>,
                          "shared_ptr element type must inherit from luable");
            if (value == nullptr)
                return this->pushnil();
            else
                return this->pushuserdata(*value);
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            static_assert(std::is_base_of_v<luable, std::remove_cv_t<std::remove_pointer_t<T>>>,
                          "pointer type must point to a type that inherits from luable");
            if (value == nullptr)
                return this->pushnil();
            else
                return this->pushuserdata(*value);
        }
        else
        {
            static_assert(std::is_base_of_v<luable, std::remove_cv_t<std::remove_reference_t<T>>>,
                          "reference type must refer to a type that inherits from luable");
            return this->pushuserdata(value);
        }
    }
    context& pushuserdata(const luable& value);
    context& push(const void* value);
    context& pop(int offset);

    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    context& pushinteger(T value)
    {
        return this->pushinteger(static_cast<lua_Integer>(value));
    }

    std::string       get_type(int offset);
    std::string       metatable(int offset);
    std::string       basetable(std::string_view metaname);
    std::string       tostring(int offset, std::string_view default_value = "");
    async::task<void> switching();
    int               tointeger(int offset, int default_value = 0);
    lua_Integer       tonumber(int offset, lua_Integer default_value = 0);
    uint64_t          touint64(int offset, uint64_t default_value = 0);
    int64_t           toint64(int offset, int64_t default_value = 0);

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
    bool toboolean(int offset, bool default_value = false);

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

        auto data = lua_touserdata(*this, offset);
        if (luable::is_weak_userdata(*this, offset))
        {
            auto shared = static_cast<std::weak_ptr<luable>*>(data)->lock();
            if constexpr (can_shared_from_this_v<T>)
                return std::static_pointer_cast<T>(shared);
            else
                return static_cast<T*>(shared.get());
        }
        else
        {
            auto raw = *static_cast<luable**>(data);
            if constexpr (can_shared_from_this_v<T>)
                return std::shared_ptr<T>(std::shared_ptr<T>(), static_cast<T*>(raw)); // non-owning
            else
                return static_cast<T*>(raw);
        }
    }

    bool is_string(int offset);
    bool is_obj(int offset);
    bool is_function(int offset);
    bool is_table(int offset);
    bool is_number(int offset);
    bool is_nil(int offset);
    int  rawgeti(int offset_t, int offset_e);
    int  rawget(int table_index);
    void rawseti(int offset_t, int offset_e);
    void settable(int table_index);
    int  rawlen(int offset_t);
    void remove(int offset);
    void new_table();
    bool next(int offset);

public:
    int                 argc();
    async::task<bool>   call(int argc, int* retc = nullptr);
    void                resume(int argc, int* n = nullptr);
    int                 yield(int retc);
    void                release();
    void                reject(std::string_view message);
    void                drop(std::string_view message = "lua context dropped");
    void                parent(context* parent);
    context*            parent() const;
    void                options(call_options opts);
    const call_options& options() const;
    void                clear_script_path();
    bool                has_dialog_slot() const;
    void                bind_dialog_slot(context*& slot);
    void                clear_dialog_slot();
    std::string_view    script_path() const;

public:
    class co_builder
    {
        friend class context;

    private:
        context&            _lua;
        fb::async_executor& _executor;

    public:
        std::optional<std::weak_ptr<fb::thread_switchable>> weak;
        std::function<async::task<void>()>                  yield;
        std::function<async::task<int>()>                   resume;

    private:
        explicit co_builder(context& lua, fb::async_executor& executor);

        static async::task<std::optional<int>> run_pipeline(fb::async_executor& executor,
                                                            std::optional<std::weak_ptr<fb::thread_switchable>> weak,
                                                            context*                                            lua_ptr,
                                                            std::function<async::task<void>()> yield_fn,
                                                            std::function<async::task<int>()>  resume_fn);

    public:
        int run();
    };

    co_builder new_co_builder();

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
    static constexpr const char* REGISTRY_KEY = "fb.root";
    static constexpr const char* MODULES_KEY  = "fb.script_modules";

public:
    using unique_lua_map = std::unordered_map<lua_State*, std::unique_ptr<thread>>;
    using bytecode_set   = std::unordered_map<std::string, std::vector<char>>;

private:
    bytecode_set                    _bytecodes;
    std::unordered_set<std::string> _lib_modules;
    int                             _loading = 0;

    void remember_lib_from_path(std::string_view path);
    void capture_loaded_libs();
    void unload_remembered_libs();

public:
    friend class context;

    unique_lua_map idle, busy;

public:
    root(fb::async_executor& executor, fb::thread& thread);
    root(const root&&) = delete;
    ~root();

public:
    root& operator= (root&)  = delete;
    root& operator= (root&&) = delete;

public:
    bool        dump(std::string_view path);
    void        invalidate(std::string_view path);
    void        unload_package(std::string_view module_name);
    bool        has_module(std::string_view path);
    bool        store_module(lua_State* L, std::string_view path);
    bool        push_module(lua_State* L, std::string_view path);
    context*    pop(context* parent, call_options options = {});
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

    void build(std::string_view name, lua_CFunction fn)
    {
        lua_register(*this, std::string(name).c_str(), fn);
    }

    void package_path(std::string_view additional_path)
    {
        lua_getglobal(*this, "package");
        lua_getfield(*this, -1, "path");
        auto current = std::string(lua_tostring(*this, -1));
        lua_pop(*this, 1);
        auto updated = current + ";" + std::string(additional_path);
        lua_pushstring(*this, updated.c_str());
        lua_setfield(*this, -2, "path");
        lua_pop(*this, 1);
    }

    void preload(std::string_view name, lua_CFunction fn)
    {
        lua_getglobal(*this, "package");
        lua_getfield(*this, -1, "preload");
        lua_pushcfunction(*this, fn);
        lua_setfield(*this, -2, std::string(name).c_str());
        lua_pop(*this, 2);
    }
};

class context_pool
{
public:
    using base_type = std::unordered_map<std::thread::id, std::unique_ptr<root>>;

private:
    base_type           _roots;
    fb::async_executor& _executor;

public:
    context_pool(fb::async_executor& executor);
    ~context_pool();

    context_pool(const context_pool&)             = delete;
    context_pool& operator= (const context_pool&) = delete;

private:
    context* new_context(context* parent = nullptr, call_options options = {});

public:
    // clang-format off
    context::guard      open(context* parent = nullptr, call_options options = {});
    context::guard      open(std::string_view path, std::string_view func, context* parent = nullptr, call_options options = {});
    async::task<void>   dump(std::string_view path);
    async::task<void>   reload_scripts(const std::vector<std::string>& relative_paths);

    base_type::iterator begin();
    base_type::iterator end();
    // clang-format on
};

class thread : public context
{
public:
    const int ref;

public:
    thread(fb::async_executor& executor, context& owner, context* parent, call_options options = {});
    thread(const thread& other) = delete;
    thread(thread&& ctx);
    ~thread();
};

void run_async(context::guard g, int argc);

} // namespace fb::lua

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
void lua_pushinteger(lua_State* L, T value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

template <class... Args>
bool fb::lua::context::load(std::string_view fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    if (fname.empty())
        return false;

    this->_script_path.clear();

    auto* root = this->owner != nullptr ? static_cast<fb::lua::root*>(this->owner) : static_cast<fb::lua::root*>(this);

#if defined DEBUG || defined _DEBUG
    const auto outermost = (root->_loading == 0);
    root->_loading++;
    if (outermost)
        root->unload_remembered_libs();

    auto ok = false;
    if (auto status = luaL_loadfile(*this, fname.c_str()); status != LUA_OK)
    {
        // A missing file is an optional hook, not an error.
        if (status != LUA_ERRFILE)
            fb::logger::warn("cannot load script {}: {}", fname, lua_tostring(*this, -1));
        lua_pop(*this, 1);
    }
    else if (lua_pcall(*this, 0, 1, 0) != LUA_OK)
    {
        const char* error = lua_tostring(*this, -1);
        fb::logger::warn("cannot load script {}: {}", fname, error != nullptr ? error : "");
        lua_pop(*this, 1);
    }
    else if (root->store_module(*this, fname) == false)
    {
        fb::logger::warn("cannot load script {}: module must return a table", fname);
    }
    else
    {
        this->_script_path = fname;
        ok                 = true;
        root->remember_lib_from_path(fname);
    }

    if (outermost)
        root->capture_loaded_libs();
    root->_loading--;
    return ok;
#else
    if (auto cached = root->_bytecodes.find(fname); cached != root->_bytecodes.end())
    {
        if (cached->second.empty() || root->has_module(fname) == false)
            return false;

        this->_script_path = fname;
        return true;
    }

    if (root->dump(fname) == false)
        return false;

    this->_script_path = fname;
    return true;
#endif
}

template <class... Args>
bool fb::lua::context::execute(std::string_view fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    if (fname.empty())
        return false;

    this->_script_path.clear();

    auto* root = this->owner != nullptr ? static_cast<fb::lua::root*>(this->owner) : static_cast<fb::lua::root*>(this);

#if defined DEBUG || defined _DEBUG
    if (luaL_loadfile(*this, fname.c_str()) != LUA_OK)
    {
        this->pop(1);
        return false;
    }

    if (lua_pcall(*this, 0, 1, 0) != LUA_OK)
    {
        this->pop(1);
        return false;
    }

    if (root->store_module(*this, fname) == false)
        return false;

    this->_script_path = fname;
    return true;
#else
    if (root->dump(fname) == false)
        return false;

    auto it = root->_bytecodes.find(fname);
    if (it == root->_bytecodes.end() || it->second.empty())
        return false;

    const auto& bytes = it->second;
    if (luaL_loadbuffer(*this, bytes.data(), bytes.size(), fname.c_str()) != LUA_OK)
    {
        this->pop(1);
        return false;
    }

    if (lua_pcall(*this, 0, 1, 0) != LUA_OK)
    {
        this->pop(1);
        return false;
    }

    if (root->store_module(*this, fname) == false)
        return false;

    this->_script_path = fname;
    return true;
#endif
}

template <class... Args>
bool fb::lua::context::dofile(std::string_view fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    if (fname.empty())
        return false;

    if (luaL_dofile(*this, fname.c_str()) != LUA_OK)
    {
        this->pop(1);
        return false;
    }

    // Discard any return values; callers only need side effects.
    lua_settop(*this, 0);
    return true;
}

template <class... Args>
bool fb::lua::context::func(std::string_view fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    if (fname.empty() || this->_script_path.empty())
        return false;

    auto* root = this->owner != nullptr ? static_cast<fb::lua::root*>(this->owner) : static_cast<fb::lua::root*>(this);

    if (root->push_module(*this, this->_script_path) == false)
        return false;

    lua_getfield(*this, -1, fname.c_str());
    lua_remove(*this, -2);
    if (lua_type(*this, -1) != LUA_TFUNCTION)
    {
        this->pop(1);
        return false;
    }
    return true;
}

#endif // !__LUA_H__
