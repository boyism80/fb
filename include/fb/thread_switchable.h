#ifndef __THREAD_SWITCHABLE_H__
#define __THREAD_SWITCHABLE_H__

#include <stdexcept>
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
 */
class thread_switchable : public lua::luable
{
public:
    LUA_PROTOTYPE

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
};

} // namespace fb

#endif