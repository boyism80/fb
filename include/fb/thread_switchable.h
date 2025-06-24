#ifndef __THREAD_SWITCHABLE_H__
#define __THREAD_SWITCHABLE_H__

/**
 * @file    thread_switchable.h
 * @brief   Base class for objects with thread affinity and safe async operations
 * @author  FB Development Team
 *
 * @details This file implements the thread_switchable base class that provides
 *          thread affinity management and safe asynchronous operations for objects
 *          in the FB 2D MMORPG server. Objects inheriting from this class can
 *          validate their thread context and participate in safe async operations
 *          using smart pointer semantics.
 *
 *          Key features:
 *          - Thread affinity validation and assertion for multi-threaded safety
 *          - Smart pointer integration with enable_shared_from_this for safe lifetime management
 *          - Weak pointer support for safe async operations without circular references
 *          - Type-safe weak pointer casting for derived class access
 *          - Lua scripting integration for dynamic object behavior
 *          - Thread context validation with proper error handling
 *          - Backward compatibility with existing raw pointer systems
 *          - Template-based type casting with compile-time safety checks
 *          - Support for both mutable and const weak pointer operations
 *          - Integration with the thread container system for proper context switching
 *
 * @note    This class is fundamental to the server's thread safety architecture
 *          and is used as a base class for most game objects that need thread
 *          affinity and safe asynchronous operation capabilities.
 */

#include <stdexcept>
#include <memory>
#include <fb/lua.h>

namespace fb {

/**
 * @brief      Forward declaration of the thread class.
 */
class thread;

/**
 * @brief      Base class for objects that can be associated with and switched between threads.
 *
 *             This class provides a foundation for objects that need to be aware of which
 *             thread they belong to and can validate thread affinity. It integrates with
 *             the Lua scripting system and provides thread safety assertions.
 *
 *             Now supports smart pointer-based lifetime management through enable_shared_from_this
 *             while maintaining backward compatibility with the existing raw pointer system.
 */
class thread_switchable : public lua::luable, public std::enable_shared_from_this<thread_switchable>
{
public:
    LUA_PROTOTYPE

private:
    using super_luable = lua::luable;
    using super_shared = std::enable_shared_from_this<thread_switchable>;

protected:
    /**
     * @brief      Constructs a new thread-switchable object with no thread association.
     *
     *             Creates an object that can be associated with threads but starts
     *             without any specific thread assignment.
     */
    thread_switchable() = default;

    /**
     * @brief      Constructs a new thread-switchable object with thread ID.
     *
     *             Creates an object associated with a specific thread identified
     *             by the given ID for thread affinity tracking.
     *
     * @param[in]  id    The identifier of the thread to associate with this object
     */
    thread_switchable(uint32_t id);

public:
    /**
     * @brief      Destroys the thread-switchable object and cleans up thread associations.
     */
    virtual ~thread_switchable() = default;

public:
    /**
     * @brief      Gets the thread associated with this thread-switchable object.
     *
     *             Returns a pointer to the thread that this object is currently
     *             associated with for execution and thread safety validation.
     *
     * @return     A pointer to the associated thread, or nullptr if not associated
     */
    virtual fb::thread* thread() const = 0;

    /**
     * @brief      Asserts that the current executing thread matches this object's thread.
     *
     *             Validates that the currently executing thread is the same as the
     *             thread associated with this object. Throws an exception if they don't match.
     *
     * @throws     std::runtime_error if the current thread doesn't match the associated thread
     */
    virtual void assert_thread() const;

    /**
     * @brief      Checks if the current executing thread matches this object's thread.
     *
     *             Performs a non-throwing check to determine if the currently executing
     *             thread is the same as the thread associated with this object.
     *
     * @return     True if the current thread matches the associated thread, false otherwise
     */
    virtual bool matched_thread() const;

    /**
     * @brief      Creates a weak pointer to this object for safe async operations.
     *
     *             Returns a weak_ptr that can be used in timer callbacks and other
     *             asynchronous operations to safely check if the object is still alive
     *             without creating circular references.
     *
     * @return     A weak_ptr to this object
     */
    std::weak_ptr<thread_switchable> weak_from_this()
    {
        return std::weak_ptr<thread_switchable>(this->shared_from_this());
    }

    /**
     * @brief      Creates a weak pointer to this object for safe async operations (const version).
     *
     *             Returns a weak_ptr that can be used in timer callbacks and other
     *             asynchronous operations to safely check if the object is still alive
     *             without creating circular references.
     *
     * @return     A weak_ptr to this object
     */
    std::weak_ptr<const thread_switchable> weak_from_this() const
    {
        return std::weak_ptr<const thread_switchable>(this->shared_from_this());
    }

    /**
     * @brief      Creates a typed weak pointer to this object for safe async operations.
     *
     *             Returns a weak_ptr of the specified derived type that can be used in
     *             timer callbacks and other asynchronous operations. This allows for
     *             type-safe access to derived class members after locking the weak_ptr.
     *
     * @tparam     T     The derived type to cast to (must inherit from thread_switchable)
     *
     * @return     A weak_ptr to this object cast to the specified type
     */
    template <typename T>
    std::weak_ptr<T> weak_from_this_as()
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<T>(this->shared_from_this());
    }

    /**
     * @brief      Creates a typed weak pointer to this object for safe async operations (const version).
     *
     *             Returns a weak_ptr of the specified derived type that can be used in
     *             timer callbacks and other asynchronous operations. This allows for
     *             type-safe access to derived class members after locking the weak_ptr.
     *
     * @tparam     T     The derived type to cast to (must inherit from thread_switchable)
     *
     * @return     A weak_ptr to this object cast to the specified type
     */
    template <typename T>
    std::weak_ptr<const T> weak_from_this_as() const
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<const T>(this->shared_from_this());
    }

    template <typename T>
    std::shared_ptr<T> shared_from_this_as()
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<T>(this->shared_from_this());
    }

    template <typename T>
    std::shared_ptr<const T> shared_from_this_as() const
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must inherit from thread_switchable");
        return std::static_pointer_cast<const T>(this->shared_from_this());
    }
};

} // namespace fb

#endif