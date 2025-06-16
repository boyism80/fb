#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <fb/dead_lock.h>

namespace fb {

/**
 * @brief      This class describes a lock error.
 */
class lock_error : public std::runtime_error
{
public:
    /**
     * @brief      Constructs a new instance.
     */
    lock_error();
    /**
     * @brief      Destroys the object.
     */
    ~lock_error() = default;
};

/**
 * @brief      This class describes a concurrent.
 */
class concurrent
{
protected:
    fb::dead_lock_detector root;

protected:
    /**
     * @brief      Constructs a new instance.
     */
    concurrent() = default;
    /**
     * @brief      Destroys the object.
     */
    ~concurrent() = default;

protected:
    /**
     * @brief      Adds the specified node.
     *
     * @param      node  The node
     */
    void add(fb::dead_lock_detector& node);

    /**
     * @brief      Checks for deadlock conditions with the specified node.
     *
     * @param[in]  node  The dead lock detector node to check.
     */
    void assert_dead_lock(const fb::dead_lock_detector& node);
};

} // namespace fb

#endif // !__CONCURRENT_H__