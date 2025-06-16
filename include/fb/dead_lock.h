#ifndef __DEAD_LOCK_H__
#define __DEAD_LOCK_H__

#include <fb/mst.h>

namespace fb {

/**
 * @brief      A deadlock detector that uses a tree structure to detect circular dependencies.
 *
 *             This class extends the MST (Multiple Search Tree) structure to detect
 *             deadlocks and circular locks in a system. It maintains a tree of lock
 *             dependencies and can detect when circular dependencies occur.
 */
class dead_lock_detector : public fb::mst<std::string>
{
public:
    /**
     * @brief      Constructs a new deadlock detector instance.
     *
     * @param[in]  data    The identifier string for this lock node.
     * @param[in]  parent  The parent node in the lock hierarchy.
     */
    dead_lock_detector(const std::string& data = "", const dead_lock_detector* parent = nullptr);

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     *
     * @param[in]  other  The other deadlock detector instance.
     */
    dead_lock_detector(const dead_lock_detector&) = delete;

    /**
     * @brief      Destroys the deadlock detector object.
     */
    ~dead_lock_detector() = default;

public:
    /**
     * @brief      Adds a node to the deadlock detection tree.
     *
     *             If the node already exists, merges its children into the existing node.
     *             Otherwise, adds it as a new node in the tree structure.
     *
     * @param      node  The deadlock detector node to add.
     */
    void add(dead_lock_detector& node);

public:
    /**
     * @brief      Asserts that no circular locks exist in the current tree.
     *
     *             Traverses the entire tree structure to detect if any circular
     *             dependencies exist. If found, throws a runtime_error with
     *             detailed information about the circular lock path.
     *
     * @throws     std::runtime_error if circular locks are detected.
     */
    void assert_circulated_lock() const;

    /**
     * @brief      Asserts that no deadlock exists between this node and the specified node.
     *
     *             Checks for potential deadlocks by examining if there are bidirectional
     *             dependencies between nodes that could lead to a deadlock situation.
     *
     * @param[in]  node  The node to check for deadlock conditions against.
     *
     * @throws     std::runtime_error if a deadlock is detected.
     */
    void assert_dead_lock(const dead_lock_detector& node) const;

public:
    /**
     * @brief      Compares this node's data with the specified value.
     *
     * @param[in]  val   The string value to compare against.
     *
     * @return     True if the node's data matches the specified value, false otherwise.
     */
    bool compare(const std::string& val) const;
};

} // namespace fb

#endif