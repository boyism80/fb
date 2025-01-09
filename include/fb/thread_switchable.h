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
     * @param[in]  id    The identifier
     */
    thread_switchable(uint32_t id);

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~thread_switchable() = default;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual fb::thread* thread() const = 0;

    /**
     * @brief      { function_description }
     */
    virtual void assert_thread() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool matched_thread() const;
};

} // namespace fb

#endif