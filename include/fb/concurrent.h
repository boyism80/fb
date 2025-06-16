#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <fb/dead_lock.h>

namespace fb {

/**
 * @brief      Exception thrown when a lock operation fails or encounters an error.
 *
 *             This exception is typically thrown when deadlock detection fails,
 *             lock acquisition times out, or other concurrency-related errors occur.
 */
class lock_error : public std::runtime_error
{
public:
    /**
     * @brief      Constructs a new lock_error exception.
     *
     *             Initializes the exception with a default error message
     *             indicating a lock operation failure.
     */
    lock_error();

    /**
     * @brief      Destroys the lock_error exception.
     */
    ~lock_error() = default;
};

/**
 * @brief      Base class for thread-safe objects with deadlock detection capabilities.
 *
 *             This class provides a foundation for implementing thread-safe operations
 *             with built-in deadlock detection. It maintains a root deadlock detector
 *             and provides methods to manage and validate lock dependencies.
 */
class concurrent
{
protected:
    fb::dead_lock_detector root;

protected:
    /**
     * @brief      Constructs a new concurrent object.
     *
     *             Initializes the deadlock detection system for this concurrent object.
     */
    concurrent() = default;

    /**
     * @brief      Destroys the concurrent object.
     *
     *             Cleans up any deadlock detection resources.
     */
    ~concurrent() = default;

protected:
    /**
     * @brief      Adds a deadlock detector node to the dependency graph.
     *
     *             Registers a new node in the deadlock detection system, establishing
     *             it as part of the lock dependency chain for this concurrent object.
     *
     * @param[in]  node  The deadlock detector node to add to the dependency graph
     */
    void add(fb::dead_lock_detector& node);

    /**
     * @brief      Validates that acquiring a lock won't create a deadlock.
     *
     *             Performs deadlock detection analysis to ensure that acquiring
     *             the lock represented by the specified node won't create a circular
     *             dependency that could lead to deadlock.
     *
     * @param[in]  node  The deadlock detector node to validate
     * @throws     lock_error if a potential deadlock is detected
     */
    void assert_dead_lock(const fb::dead_lock_detector& node);
};

} // namespace fb

#endif // !__CONCURRENT_H__