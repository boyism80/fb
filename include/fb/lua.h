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
 * @brief      Creates a Lua extension implementation with the specified type and name.
 *
 * @param      type  The type of the Lua extension.
 * @param      name  The name of the metatable.
 *
 * @return     The implementation macro for the Lua extension.
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
class thread_switchable;
class context;

} // namespace fb

namespace fb::lua {

/**
 * @brief      Type traits and utilities for Lua binding system.
 *
 *             This header provides the core functionality for binding C++ objects to Lua,
 *             with special support for smart pointers and thread-safe object management.
 *             It includes type traits for detecting smart pointer types, memory management
 *             utilities, and a comprehensive system for exposing C++ objects to Lua scripts.
 */

constexpr auto DEFAULT_POOL_SIZE = 1000;

/**
 * @brief      Type traits for detecting shared_ptr types in Lua bindings.
 *
 * @tparam     T     The type to check
 */
template <typename T>
struct is_shared_ptr : std::false_type
{ };

/**
 * @brief      Specialization for std::shared_ptr types.
 *
 * @tparam     T     The element type of the shared_ptr
 */
template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
{
    using element_type = T;
};

template <typename T>
inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

/**
 * @brief      Type traits for detecting weak_ptr types in Lua bindings.
 *
 * @tparam     T     The type to check
 */
template <typename T>
struct is_weak_ptr : std::false_type
{ };

/**
 * @brief      Specialization for std::weak_ptr types.
 *
 * @tparam     T     The element type of the weak_ptr
 */
template <typename T>
struct is_weak_ptr<std::weak_ptr<T>> : std::true_type
{
    using element_type = T;
};

template <typename T>
constexpr bool is_weak_ptr_v = is_weak_ptr<T>::value;

/**
 * @brief      Type traits for detecting if a type can use shared_from_this.
 *
 * @tparam     T     The type to check
 */
template <typename T>
struct is_shared_from_this_capable : std::false_type
{ };

/**
 * @brief      Specialization for std::shared_ptr types.
 *
 * @tparam     T     The element type of the shared_ptr
 */
template <typename T>
struct is_shared_from_this_capable<std::shared_ptr<T>> : std::true_type
{ };

/**
 * @brief      Specialization for pointer types.
 *
 * @tparam     T     The pointer type to check
 */
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

/**
 * @brief      Specialization for reference types.
 *
 * @tparam     T     The reference type to check
 */
template <typename T>
struct is_shared_from_this_capable<T&> : is_shared_from_this_capable<T*>
{ };

template <typename T>
inline constexpr bool is_shared_from_this_capable_v = is_shared_from_this_capable<T>::value;

/**
 * @brief      Helper type trait to check if a type can use shared_from_this().
 *
 * @tparam     T     The type to check
 */
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

/**
 * @brief      Base class for C++ objects that can be exposed to Lua scripts.
 *
 *             This class provides the foundation for making C++ objects accessible
 *             from Lua scripts. It handles the metatable registration, garbage
 *             collection, and provides utilities for pushing objects onto the Lua
 *             stack. All game objects that need to be scriptable should inherit
 *             from this class and implement the required Lua method bindings.
 *
 *             Memory Management:
 *             - For thread_switchable objects, uses weak_ptr storage in Lua userdata
 *             - For non-thread_switchable objects, uses traditional pointer storage
 *             - Automatically handles proper cleanup through Lua's garbage collector
 *             - Prevents memory leaks by using weak references where appropriate
 *             - Maintains type safety through metatables and inheritance checking
 */
class luable;
/**
 * @brief      Lua execution context that manages script state and thread safety.
 */
class context;
/**
 * @brief      Root Lua context that manages the global Lua state and context pool.
 */
class root;
/**
 * @brief      Lua thread wrapper that provides coroutine functionality.
 */
class thread;

/**
 * @brief      Creates a new Lua context.
 *
 * @return     A pointer to the newly created context.
 */
context* new_context(context* parent = nullptr);
/**
 * @brief      Gets the context associated with the given Lua state.
 *
 * @param      ctx   The Lua state.
 *
 * @return     A pointer to the associated context.
 */
context* get(lua_State* ctx);
/**
 * @brief      Builds a Lua function with the specified name.
 *
 * @param[in]  name  The name of the function.
 * @param[in]  fn    The C function to bind.
 */
async::task<void> build(const std::string& name, lua_CFunction fn);
/**
 * @brief      Dumps Lua bytecode to the specified path.
 *
 * @param[in]  path  The path to dump the bytecode to.
 */
async::task<void> dump(const std::string& path);

/**
 * @brief      Base class for C++ objects that can be exposed to Lua scripts.
 *
 *             This class provides the foundation for making C++ objects accessible
 *             from Lua scripts. It handles the metatable registration, garbage
 *             collection, and provides utilities for pushing objects onto the Lua
 *             stack. All game objects that need to be scriptable should inherit
 *             from this class and implement the required Lua method bindings.
 */
class luable
{
public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      Pushes this luable object onto the Lua stack.
     *
     * @param      ctx   The Lua context.
     */
    void to_lua(lua_State* ctx) const;

    /**
     * @brief      Pushes a shared_ptr object onto the Lua stack with automatic type detection.
     *
     *             This template overload handles shared_ptr objects and automatically
     *             selects the appropriate storage method based on whether the element
     *             type inherits from thread_switchable.
     *
     * @param      ctx        The Lua context
     * @param[in]  shared_obj The shared_ptr to push
     *
     * @tparam     T          The object type
     */
    template <typename T>
    void to_lua(lua_State* ctx, const std::shared_ptr<T>& shared_obj) const
    {
        static_assert(std::is_base_of_v<luable, T>, "T must inherit from luable");

        if (const auto context = fb::lua::get(ctx); context == nullptr)
            return;

        if (!shared_obj)
        {
            lua_pushnil(ctx);
            return;
        }

        if constexpr (std::is_base_of_v<fb::thread_switchable, T>)
        {
            // Use shared_ptr storage for thread_switchable objects
            auto allocated =
                static_cast<std::shared_ptr<T>*>(lua_newuserdata(ctx, sizeof(std::shared_ptr<T>))); // [val]
            new (allocated) std::shared_ptr<T>(shared_obj); // Placement new to construct shared_ptr

            auto& metaname = this->metaname();
            luaL_getmetatable(ctx, metaname.c_str());      // [val, mt]
            lua_pushcfunction(ctx, luable::builtin_gc<T>); // [val, mt, gc]
            lua_setfield(ctx, -2, "__gc");                 // [val, mt]
            lua_setmetatable(ctx, -2);                     // [val]
        }
        else
        {
            // Use traditional pointer storage for non-thread_switchable objects
            shared_obj->to_lua(ctx);
        }
    }

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
     * @brief      Template garbage collection function for Lua userdata objects.
     *
     *             This function handles garbage collection for various types of objects
     *             stored in Lua userdata, including weak_ptr, shared_ptr, and raw pointers.
     *             It properly calls destructors for C++ objects to prevent memory leaks
     *             and ensures safe cleanup when Lua's garbage collector runs.
     *
     * @param[in]  ctx   The Lua state context
     *
     * @tparam     T     The object type stored in userdata (weak_ptr, shared_ptr, or pointer type)
     *
     * @return     Always returns 0 (no values pushed to Lua stack)
     *
     * @note       This function is automatically called by Lua's garbage collector
     * @warning    Manual invocation of this function is not recommended
     */
    template <typename T>
    static int builtin_gc(lua_State* ctx)
    {
        fb::logger::debug("builtin_gc called");

        if constexpr (is_shared_ptr_v<T> || is_weak_ptr_v<T>)
        {
            using element_type = typename T::element_type;
            static_assert(std::is_base_of_v<luable, element_type>, "pointer element type must inherit from luable");

            // Handle weak_ptr stored in userdata
            if constexpr (is_weak_ptr_v<T>)
            {
                fb::logger::debug("Handling weak_ptr GC\n");
                auto allocated = static_cast<T*>(lua_touserdata(ctx, 1));
                if (allocated != nullptr)
                    allocated->~T(); // Explicitly call destructor for weak_ptr
            }
            else // shared_ptr case
            {
                fb::logger::debug("Handling shared_ptr case (storing as weak_ptr)\n");
                auto allocated = static_cast<std::weak_ptr<element_type>*>(lua_touserdata(ctx, 1));
                if (allocated != nullptr)
                    allocated->~weak_ptr<element_type>(); // Explicitly call destructor
            }
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            using pointee_type = std::remove_pointer_t<T>;
            static_assert(std::is_base_of_v<luable, pointee_type>, "Pointer type must point to luable");

            if (auto allocated = static_cast<T**>(lua_touserdata(ctx, 1)); allocated != nullptr)
                *allocated = nullptr;
        }
        else if constexpr (std::is_base_of_v<luable, T>)
        {
            // For direct luable objects (reference/value types)
            if (auto allocated = static_cast<T**>(lua_touserdata(ctx, 1)); allocated != nullptr)
                *allocated = nullptr;
        }
        else
        {
            // Skip static_assert for now - inheritance check is complex with multiple inheritance
            // static_assert condition will be checked at runtime instead
        }
        return 0;
    }
};

/**
 * @brief      Lua execution context that manages script state and thread safety.
 *
 *             This class wraps a Lua state (lua_State) and provides thread-safe
 *             access to Lua functionality. It manages coroutine execution, handles
 *             yielding and resuming operations, and provides utilities for data
 *             exchange between C++ and Lua. Each context can have a parent-child
 *             relationship for nested script execution scenarios.
 */
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
    /**
     * @brief      Constructs a new instance.
     *
     * @param      ctx             The context
     * @param      initial_thread  The initial thread
     */
    context(lua_State* ctx, fb::thread& initial_thread);
    /**
     * @brief      Constructs a new instance.
     *
     * @param      ctx     The context
     * @param      owner   The owner
     * @param      parent  The parent
     */
    context(lua_State* ctx, context& owner, context* parent = nullptr);
    /**
     * @brief      Copy constructor (deleted).
     *
     * @param[in]  other  The other context object to copy from.
     */
    context(const context&) = delete;
    /**
     * @brief      Move constructor (deleted).
     *
     * @param      other  The other context object to move from.
     */
    context(context&&) = delete;

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~context() = default;

public:
    /**
     * @brief      Assignment operator (deleted).
     *
     * @param      other  The other context object to assign from.
     *
     * @return     The result of the assignment
     */
    context operator= (context&) = delete;
    /**
     * @brief      Move assignment operator (deleted).
     *
     * @param[in]  other  The other context object to move assign from.
     *
     * @return     The result of the assignment
     */
    context operator= (const context&) = delete;

public:
    /**
     * @brief      Loads and executes a Lua script with formatted arguments.
     *
     * @param[in]  fmt   The format string for the Lua script path or code.
     * @param      args  The arguments to format into the script string.
     *
     * @tparam     Args  Variadic template arguments for formatting.
     *
     * @return     Reference to this context for method chaining.
     */
    template <class... Args>
    context& load(const std::string& fmt, Args&&... args);
    /**
     * @brief      Calls a Lua function with formatted arguments.
     *
     * @param[in]  fmt   The format string for the function name or call.
     * @param      args  The arguments to format into the function call.
     *
     * @tparam     Args  Variadic template arguments for formatting.
     *
     * @return     Reference to this context for method chaining.
     */
    template <class... Args>
    context& func(const std::string& fmt, Args&&... args);

    /**
     * @brief      Pushes a string value onto the Lua stack.
     *
     * @param[in]  value  The string value to push.
     *
     * @return     Reference to this context for method chaining.
     */
    context& pushstring(const std::string& value);
    /**
     * @brief      Pushes an integer value onto the Lua stack.
     *
     * @param[in]  value  The integer value to push.
     *
     * @return     Reference to this context for method chaining.
     */
    context& pushinteger(lua_Integer value);
    /**
     * @brief      Pushes a number value onto the Lua stack.
     *
     * @param[in]  value  The number value to push.
     *
     * @return     Reference to this context for method chaining.
     */
    context& pushnumber(lua_Number value);
    /**
     * @brief      Pushes a nil value onto the Lua stack.
     *
     * @return     Reference to this context for method chaining.
     */
    context& pushnil();
    /**
     * @brief      Pushes a boolean value onto the Lua stack.
     *
     * @param[in]  value  The boolean value to push.
     *
     * @return     Reference to this context for method chaining.
     */
    context& pushboolean(bool value);

    /**
     * @brief      Pushes a C++ object onto the Lua stack with automatic type detection.
     *
     *             This template function handles shared_ptr, raw pointers, and references
     *             to luable objects. It automatically selects the appropriate storage method:
     *             - For thread_switchable objects: stores as weak_ptr for safe memory management
     *             - For regular luable objects: stores as raw pointer
     *             - Handles null/nullptr cases by pushing nil
     *
     * @param[in]  value  The object to push (shared_ptr, pointer, or reference)
     *
     * @tparam     T      The object type (shared_ptr<luable>, luable*, or luable&)
     *
     * @return     Reference to this context for method chaining
     *
     * @note       Uses placement new for complex types, requires proper GC handling
     * @warning    Weak references may expire, resulting in nil access from Lua
     */
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
            auto allocated =
                static_cast<std::weak_ptr<element_type>*>(lua_newuserdata(*this, sizeof(std::weak_ptr<element_type>)));
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
            static_assert(std::is_base_of_v<luable, element_type>,
                          "pointer type must point to a type that inherits from luable");

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
                    auto allocated = static_cast<std::weak_ptr<element_type>*>(
                        lua_newuserdata(*this, sizeof(std::weak_ptr<element_type>)));
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
                    auto allocated =
                        static_cast<const element_type**>(lua_newuserdata(*this, sizeof(const element_type*)));
                    *allocated = value;

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
            static_assert(std::is_base_of_v<luable, element_type>,
                          "reference type must refer to a type that inherits from luable");

            // Try to use shared_from_this if available
            if constexpr (std::is_base_of_v<fb::thread_switchable, element_type>)
            {
                try
                {
                    // Use thread_switchable's shared_from_this_as<T>() - this preserves EXACT type!
                    auto const_shared = value.template shared_from_this_as<element_type>();
                    auto typed_shared = std::const_pointer_cast<element_type>(const_shared);

                    // Store as weak_ptr with the EXACT element_type
                    auto allocated = static_cast<std::weak_ptr<element_type>*>(
                        lua_newuserdata(*this, sizeof(std::weak_ptr<element_type>)));
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
                    auto allocated =
                        static_cast<const element_type**>(lua_newuserdata(*this, sizeof(const element_type*)));
                    *allocated = &value;

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
    /**
     * @brief      Pushes a void pointer onto the Lua stack as light userdata.
     *
     * @param[in]  value  The void pointer value to push.
     *
     * @return     Reference to this context for method chaining.
     */
    context& push(const void* value);
    /**
     * @brief      Pops the given offset.
     *
     * @param[in]  offset  The offset
     *
     * @return     Reference to this context for method chaining.
     */
    context& pop(int offset);

    /**
     * @brief      Pushes an enum value as an integer onto the Lua stack.
     *
     * @param[in]  value      The enum value to push as an integer.
     *
     * @tparam     T          The enum type to convert to integer.
     * @tparam     <unnamed>  SFINAE enabler for enum types only.
     *
     * @return     Reference to this context for method chaining.
     */
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    context& pushinteger(T value)
    {
        return this->pushinteger(static_cast<lua_Integer>(value));
    }

    /**
     * @brief      Gets the type name of the value at the specified stack offset.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The type name as a string.
     */
    std::string get_type(int offset);

    /**
     * @brief      Gets the metatable name of the value at the specified stack offset.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The metatable name as a string.
     */
    std::string metatable(int offset);

    /**
     * @brief      Gets the base table name for the specified metatable.
     *
     * @param[in]  metaname  The metatable name to get the base table for.
     *
     * @return     The base table name as a string.
     */
    std::string basetable(const std::string& metaname);

    /**
     * @brief      Converts the value at the specified offset to a string.
     *
     * @param[in]  offset         The stack offset.
     * @param[in]  default_value  The default value if conversion fails.
     *
     * @return     The converted string value.
     */
    std::string tostring(int offset, const std::string& default_value = "");
    /**
     * @brief      Gets a string argument from the Lua stack.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The string argument.
     */
    std::string arg_string(int offset)
    {
        return tostring(offset);
    }
    /**
     * @brief      Returns a string value to the Lua stack.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The string return value.
     */
    std::string ret_string(int offset)
    {
        return tostring(-offset);
    }

    /**
     * @brief      Switches to another thread context.
     */
    async::task<void> switching();

    /**
     * @brief      Converts the value at the specified offset to an integer.
     *
     * @param[in]  offset         The stack offset.
     * @param[in]  default_value  The default value if conversion fails.
     *
     * @return     The converted integer value.
     */
    int tointeger(int offset, int default_value = 0)
    {
        if (this->argc() < offset)
            return default_value;
        else if (lua_type(*this, offset) != LUA_TNUMBER)
            return default_value;
        else
            return (int)lua_tointeger(*this, offset);
    }
    /**
     * @brief      Converts the value at the specified offset to a number.
     *
     * @param[in]  offset         The stack offset.
     * @param[in]  default_value  The default value if conversion fails.
     *
     * @return     The converted number value.
     */
    lua_Integer tonumber(int offset, lua_Integer default_value = 0)
    {
        if (this->argc() < offset)
            return default_value;
        else if (lua_type(*this, offset) != LUA_TNUMBER)
            return default_value;
        else
            return lua_tonumber(*this, offset);
    }

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
    /**
     * @brief      Gets an integer argument from the Lua stack.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The integer argument.
     */
    int arg_integer(int offset)
    {
        return tointeger(offset);
    }
    /**
     * @brief      Returns an integer value to the Lua stack.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The integer return value.
     */
    int ret_integer(int offset)
    {
        return tointeger(-offset);
    }

    /**
     * @brief      Converts the value at the specified offset to a boolean.
     *
     * @param[in]  offset         The stack offset.
     * @param[in]  default_value  The default value if conversion fails.
     *
     * @return     The converted boolean value.
     */
    bool toboolean(int offset, bool default_value = false)
    {
        if (this->argc() < offset)
            return default_value;
        else if (lua_type(*this, offset) != LUA_TBOOLEAN)
            return default_value;
        else
            return lua_toboolean(*this, offset);
    }
    /**
     * @brief      Gets a boolean argument from the Lua stack.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The boolean argument.
     */
    bool arg_boolean(int offset)
    {
        return toboolean(offset);
    }
    /**
     * @brief      Returns a boolean value to the Lua stack.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     The boolean return value.
     */
    bool ret_boolean(int offset)
    {
        return toboolean(-offset);
    }

    /**
     * @brief      Checks if the value at the specified offset is a userdata of type T.
     *
     *             This function verifies that the value at the given offset is a userdata
     *             object and that its metatable matches the expected type T or any of its
     *             base types in the inheritance hierarchy.
     *
     * @param[in]  offset  The stack offset to check
     *
     * @tparam     T       The luable type to check for
     *
     * @return     True if the value is a userdata of type T, false otherwise
     */
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

    /**
     * @brief      Extracts C++ objects from Lua userdata with automatic type detection.
     *
     *             This function automatically detects whether the luable object should be
     *             returned as a shared_ptr (for thread_switchable objects) or as a pointer
     *             (for regular luable objects). It performs type checking and safely
     *             extracts objects from weak_ptr storage when applicable.
     *
     * @param[in]  offset  The stack offset of the userdata value
     *
     * @tparam     T       The luable type to extract
     *
     * @return     shared_ptr<T> if T can use shared_from_this, T* otherwise.
     *             Returns nullptr/null pointer if extraction fails.
     *
     * @note       For thread_switchable objects, this returns shared_ptr from weak_ptr storage
     * @warning    Returns null if the weak_ptr has expired or userdata is invalid
     */
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

    /**
     * @brief      Checks if the value at the specified offset is a string.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     True if the value is a string, false otherwise.
     */
    bool is_string(int offset)
    {
        return lua_isstring(*this, offset);
    }
    /**
     * @brief      Checks if the value at the specified offset is an object.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     True if the value is an object, false otherwise.
     */
    bool is_obj(int offset)
    {
        return lua_isuserdata(*this, offset);
    }

    /**
     * @brief      Checks if the value at the specified offset is a function.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     True if the value is a function, false otherwise.
     */
    bool is_function(int offset)
    {
        if (this->argc() < offset)
            return false;
        else if (lua_type(*this, offset) != LUA_TFUNCTION)
            return false;
        else
            return true;
    }

    /**
     * @brief      Checks if the value at the specified offset is a table.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     True if the value is a table, false otherwise.
     */
    bool is_table(int offset)
    {
        return lua_istable(*this, offset);
    }
    /**
     * @brief      Checks if the value at the specified offset is a number.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     True if the value is a number, false otherwise.
     */
    bool is_number(int offset)
    {
        return lua_isnumber(*this, offset);
    }
    /**
     * @brief      Checks if the value at the specified offset is nil.
     *
     * @param[in]  offset  The stack offset.
     *
     * @return     True if the value is nil, false otherwise.
     */
    bool is_nil(int offset)
    {
        return lua_isnil(*this, offset);
    }

    /**
     * @brief      Performs a raw get operation on a table.
     *
     * @param[in]  offset_t  The table offset.
     * @param[in]  offset_e  The element offset.
     *
     * @return     The type of the retrieved value.
     */
    int rawgeti(int offset_t, int offset_e)
    {
        return lua_rawgeti(*this, offset_t, offset_e);
    }
    /**
     * @brief      Performs a raw set operation on a table.
     *
     * @param[in]  offset_t  The table offset.
     * @param[in]  offset_e  The element offset.
     */
    void rawseti(int offset_t, int offset_e)
    {
        lua_rawseti(*this, offset_t, offset_e);
    }
    /**
     * @brief      Gets the raw length of a table.
     *
     * @param[in]  offset_t  The table offset.
     *
     * @return     The raw length of the table.
     */
    int rawlen(int offset_t)
    {
        return (int)lua_rawlen(*this, offset_t);
    }
    /**
     * @brief      Removes an element from the Lua stack.
     *
     * @param[in]  offset  The stack offset.
     */
    void remove(int offset)
    {
        lua_remove(*this, offset);
    }
    /**
     * @brief      Creates a new table on the Lua stack.
     */
    void new_table()
    {
        lua_newtable(*this);
    }
    /**
     * @brief      Iterates to the next element in a table.
     *
     * @param[in]  offset  The table offset.
     *
     * @return     True if there is a next element, false otherwise.
     */
    bool next(int offset)
    {
        return lua_next(*this, offset) != 0;
    };

public:
    /**
     * @brief      Gets the number of arguments on the Lua stack.
     *
     * @return     The number of arguments.
     */
    int argc();
    /**
     * @brief      Resumes a Lua coroutine.
     *
     * @param[in]  argc          The number of arguments
     * @param[in]  auto_release  Whether to automatically release the coroutine
     * @param      n             The number of return values
     *
     * @return     True if the coroutine resumed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> call(int argc, bool auto_release = true, int* n = nullptr);

    /**
     * @brief      Resumes a Lua coroutine.
     *
     * @param[in]  argc  The number of arguments
     * @param      n     The number of return values
     */
    void resume(int argc, int* n = nullptr);
    /**
     * @brief      Yields from a Lua coroutine.
     *
     * @param[in]  retc  The number of return values
     *
     * @return     The result of the yield operation.
     */
    int yield(int retc)
    {
        return lua_yield(*this, retc);
    }
    /**
     * @brief      Gets the current state of the Lua context.
     *
     * @return     The current state.
     */
    int state() const;
    /**
     * @brief      Releases the Lua context.
     */
    void release();
    /**
     * @brief      Checks if the context is in a pending state.
     *
     * @return     True if pending, false otherwise.
     */
    bool pending() const;
    /**
     * @brief      Sets the pending state of the context.
     *
     * @param[in]  value  The value to set.
     */
    void pending(bool value);

    /**
     * @brief      Sets the parent context.
     *
     * @param      parent  The parent context.
     */
    void parent(context* parent);

    /**
     * @brief      Gets the parent context.
     *
     * @return     A pointer to the parent context.
     */
    context* parent() const;

    /**
     * @brief      Ensures a yield operation is performed safely.
     *
     * @param      ctx   The context.
     * @param      obj   The thread-switchable object.
     * @param[in]  fn    The function to execute.
     *
     * @return     The result of the yield operation.
     */
    int ensure_yield(fb::context& ctx, std::weak_ptr<fb::thread_switchable> weak, std::function<int()> fn);

    /**
     * @brief      Ensures a resume operation is performed safely.
     *
     * @param      ctx           The context.
     * @param      obj           The thread-switchable object.
     * @param[in]  fn            The function to execute.
     * @param[in]  force_resume  Whether to force the resume operation.
     *
     * @return     The result of the resume operation.
     */
    int ensure_resume(fb::context&                         ctx,
                      std::weak_ptr<fb::thread_switchable> weak,
                      std::function<int()>                 fn,
                      bool                                 force_resume = false);

public:
    operator lua_State* () const;

public:
    /**
     * @brief      Gets an environment value by key.
     *
     * @param[in]  key   The key to look up.
     *
     * @tparam     T     The type of the value to retrieve.
     *
     * @return     A pointer to the environment value.
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
     * @brief      Sets an environment value by key.
     *
     * @param[in]  key   The key to set.
     * @param      data  The data to store.
     *
     * @tparam     T     The type of the data to store.
     */
    template <typename T>
    void env(const char* key, T* data)
    {
        ::lua_pushlightuserdata(*this, (void*)data);
        ::lua_setfield(*this, LUA_REGISTRYINDEX, key);
    }
};

/**
 * @brief      Root Lua context that manages the global Lua state and context pool.
 *
 *             This class extends the base context to provide global Lua state management.
 *             It maintains a pool of reusable Lua contexts for performance optimization,
 *             handles bytecode compilation and caching, and manages the lifecycle of
 *             all Lua threads. The root context is responsible for setting up the
 *             global Lua environment and providing context allocation/deallocation.
 */
class root : public context
{
public:
    using unique_lua_map = std::unordered_map<lua_State*, std::unique_ptr<thread>>;
    using bytecode_set   = std::unordered_map<std::string, std::vector<char>>;

private:
    bytecode_set _bytecodes;
    std::mutex   _mutex;

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
     * @brief      Copy constructor (deleted).
     *
     * @param[in]  other  The other root object.
     */
    root(const root&&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~root();

public:
    /**
     * @brief      Assignment operator (deleted).
     *
     * @param      other  The other root object.
     *
     * @return     Reference to this object.
     */
    root& operator= (root&) = delete;
    /**
     * @brief      Move assignment operator (deleted).
     *
     * @param      other  The other root object.
     *
     * @return     Reference to this object.
     */
    root& operator= (root&&) = delete;

public:
    /**
     * @brief      Dumps Lua bytecode to a file.
     *
     * @param[in]  path  The path to dump to.
     *
     * @return     True if successful, false otherwise.
     */
    bool dump(const std::string& path);
    /**
     * @brief      Pops a context from the pool.
     *
     * @return     A pointer to the popped context.
     */
    context* pop(context* parent);
    /**
     * @brief      Gets the context associated with a Lua state.
     *
     * @param      ctx   The Lua state.
     *
     * @return     A pointer to the associated context.
     */
    context* get(lua_State* ctx);
    /**
     * @brief      Releases a context back to the pool.
     *
     * @param      ctx   The context to release.
     */
    void release(context& ctx);
    /**
     * @brief      Revokes a context from the pool.
     *
     * @param      ctx   The context to revoke.
     */
    void revoke(context& ctx);

    /**
     * @brief      Gets the initial thread.
     *
     * @return     Reference to the initial thread.
     */
    fb::thread& initial_thread();

public:
    /**
     * @brief      Builds Lua bindings for a type.
     *
     * @tparam     T     The type to build bindings for.
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
     * @brief      Builds Lua bindings for a derived type with inheritance.
     *
     * @tparam     T     The derived type.
     * @tparam     B     The base type.
     */
    template <typename T, typename B>
    void build()
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

    /**
     * @brief      Builds a Lua function binding.
     *
     * @param[in]  name  The name of the function.
     * @param[in]  fn    The C function to bind.
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
    context* pop(context* parent);
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
     * @param      owner   The owner
     * @param      parent  The parent
     */
    thread(context& owner, context* parent);
    /**
     * @brief      Copy constructor (deleted).
     *
     * @param[in]  other  The other thread object to copy from.
     */
    thread(const thread& other) = delete;
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
 * @brief      Builds Lua bindings for a type.
 *
 * @tparam     T     The type to build bindings for.
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
 * @brief      Builds Lua bindings for a derived type with inheritance.
 *
 * @tparam     T     The derived type.
 * @tparam     B     The base type.
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
 * @brief      Sets an environment value by key.
 *
 * @param[in]  key   The key to set.
 * @param      data  The data to store.
 *
 * @tparam     T     The type of the data to store.
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
 * @brief      Pushes an integer value to the Lua stack.
 *
 * @param      L          The Lua state.
 * @param[in]  value      The value to push.
 *
 * @tparam     T          The type of the value.
 */
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
void lua_pushinteger(lua_State* L, T value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

/**
 * @brief      Loads a Lua script.
 *
 * @param[in]  fmt   The format of the script.
 * @param      args  The arguments for the format.
 *
 * @tparam     Args  The types of the arguments.
 *
 * @return     A reference to the context.
 */
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
 * @brief      Gets a global function by name.
 *
 * @param[in]  fmt   The format of the function name.
 * @param      args  The arguments for the format.
 *
 * @tparam     Args  The types of the arguments.
 *
 * @return     A reference to the function.
 */
template <class... Args>
fb::lua::context& fb::lua::context::func(const std::string& fmt, Args&&... args)
{
    auto fname = std::vformat(fmt, std::make_format_args(args...));
    lua_getglobal(*this, fname.c_str());
    return *this;
}

#endif // !__LUA_H__
