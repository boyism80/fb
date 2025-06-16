#ifndef __THREAD_SWITCHABLE_H__
#define __THREAD_SWITCHABLE_H__

#include <stdexcept>
#include <fb/lua.h>

namespace fb {

/**
 * @brief      This class describes a thread.
 */
class thread;

/**
 * @brief      This class describes a thread switchable.
 */
class thread_switchable : public lua::luable
{
public:
    LUA_PROTOTYPE

protected:
    /**
     * @brief      Constructs a new instance.
     */
    thread_switchable() = default;

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  id    The ID of the thread.
     */
    thread_switchable(uint32_t id);

public:
    /**
     * @brief      Destroys the thread switchable.
     */
    virtual ~thread_switchable() = default;

public:
    /**
     * @brief      Gets the thread associated with this thread switchable.
     *
     * @return     A pointer to the associated thread.
     */
    virtual fb::thread* thread() const = 0;

    /**
     * @brief      Asserts that the current thread matches the thread switchable's thread.
     */
    virtual void assert_thread() const;

    /**
     * @brief      Checks if the current thread matches the thread switchable's thread.
     *
     * @return     True if the threads match, false otherwise.
     */
    virtual bool matched_thread() const;
};

} // namespace fb

#endif