#ifndef __MST_H__
#define __MST_H__

#include <queue>
#include <list>
#include <set>
#include <boost/algorithm/string/join.hpp>
#include <fb/logger.h>

namespace fb {

/**
 * @brief      A Multiple Search Tree (MST) implementation for hierarchical data structures.
 *
 *             This template class provides a tree-based data structure that supports
 *             multiple children per node, tree traversal, searching, and containment
 *             checking. It's designed to be used as a base class for specialized
 *             tree structures like deadlock detectors.
 *
 * @tparam     T     The type of data stored in each node.
 */
template <typename T>
class mst
{
public:
    /**
     * @brief      Type alias for a list of node pointers.
     */
    using node_list = std::vector<mst<T>*>;

    /**
     * @brief      Type alias for a route through the tree (list of node references).
     */
    using node_route = std::list<std::reference_wrapper<const mst<T>>>;

    /**
     * @brief      Type alias for initialization function.
     */
    using init_fn = std::function<const T&()>;

private:
    std::vector<std::unique_ptr<fb::mst<T>>> _allocated;
    node_list                                _nodes;

public:
    /**
     * @brief      The data stored in this node.
     */
    const T data;

    /**
     * @brief      Pointer to the parent node (nullptr for root nodes).
     */
    const fb::mst<T>* parent = nullptr;

protected:
    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    mst(const mst<T>&) = delete;

    /**
     * @brief      Constructs an MST node with data and optional parent.
     *
     * @param[in]  data    The data to store in this node.
     * @param[in]  parent  The parent node (nullptr for root nodes).
     */
    mst(const T& data, const mst<T>* parent) :
        data(data),
        parent(parent)
    { }

public:
    /**
     * @brief      Virtual destructor for proper inheritance cleanup.
     */
    virtual ~mst() = default;

protected:
    /**
     * @brief      Checks if this subtree contains the specified node structure.
     *
     *             Performs a structural comparison to determine if the given node
     *             and its subtree structure exists within this subtree.
     *
     * @param[in]  node  The node structure to search for.
     *
     * @return     True if the node structure is contained, false otherwise.
     */
    bool contains(const fb::mst<T>& node) const
    {
        if (this->data != node.data)
            return false;

        if (this->_nodes.size() < node._nodes.size())
            return false;

        if (node._nodes.size() == 0)
            return true;

        for (auto i1 = this->_nodes.cbegin(); i1 != this->_nodes.cend(); i1++)
        {
            auto match = true;
            for (auto i2 = node._nodes.cbegin(); i2 != node._nodes.cend(); i2++)
            {
                match = match && (*i1)->contains(i2->get());
                if (!match)
                    break;
            }

            if (match)
                return true;
        }

        return false;
    }

protected:
    /**
     * @brief      Checks if the specified subtree exists anywhere in this tree.
     *
     *             Recursively searches through all nodes to find if the given
     *             subtree structure exists at any level.
     *
     * @param[in]  sub   The subtree to search for.
     *
     * @return     True if the subtree is found, false otherwise.
     */
    bool subtree(const fb::mst<T>& sub) const
    {
        if (this->contains(sub))
            return true;

        for (auto i1 = this->_nodes.cbegin(); i1 != this->_nodes.cend(); i1++)
        {
            if ((*i1)->subtree(sub))
                return true;
        }

        return false;
    }

protected:
    /**
     * @brief      Traverses the tree and calls a function for each complete path.
     *
     *             Performs a breadth-first traversal of the tree, calling the
     *             provided function for each path from root to leaf. The traversal
     *             can be terminated early by returning true from the function.
     *
     * @param[in]  fn    Function to call for each path (returns true to stop).
     */
    void travel(const std::function<bool(const node_route&)>& fn) const
    {
        auto queue = std::queue<fb::mst<T>::node_route>();
        if (this->data.empty())
        {
            for (auto& node : this->_nodes)
                queue.push({*node});
        }
        else
        {
            queue.push({*this});
        }

        while (queue.empty() == false)
        {
            auto nodes = fb::mst<T>::node_route(queue.front());
            queue.pop();

            auto& back = nodes.back().get();
            if (back._nodes.empty())
            {
                if (fn(nodes) == true)
                    break;
            }
            else
            {
                for (auto& child : back._nodes)
                {
                    auto route = fb::mst<T>::node_route(nodes);
                    route.push_back(*child);
                    queue.push(route);
                }
            }
        }
    }

public:
    /**
     * @brief      Creates and adds a new node of type R with the given arguments.
     *
     *             Constructs a new node using the provided arguments, takes ownership
     *             of it, and adds it to this node's children.
     *
     * @param[in]  args  Arguments to forward to R's constructor.
     *
     * @tparam     R     The type of node to create (must inherit from mst<T>).
     * @tparam     Args  Parameter pack for constructor arguments.
     *
     * @return     Reference to the newly created and added node.
     */
    template <typename R, typename... Args>
    R& add(Args&&... args)
    {
        auto ptr  = std::unique_ptr<fb::mst<T>>(new R(std::forward<Args>(args)...));
        auto node = ptr.get();
        this->_allocated.push_back(std::move(ptr));

        this->add(*node);
        return static_cast<R&>(*node);
    }

public:
    /**
     * @brief      Adds an existing node as a child of this node.
     *
     * @param      node  The node to add as a child.
     *
     * @tparam     R     The type of the node being added.
     *
     * @return     Reference to this node for method chaining.
     */
    template <typename R> R& add(R& node)
    {
        this->_nodes.push_back(&node);
        return static_cast<R&>(*this);
    }

public:
    /**
     * @brief      Gets the root node of the tree.
     *
     *             Traverses up the parent chain to find the topmost node.
     *
     * @return     Reference to the root node.
     */
    const mst<T>& root() const
    {
        const mst* node = this;
        while (node->parent != nullptr)
        {
            node = node->parent;
        }

        return *node;
    }

public:
    /**
     * @brief      Searches for a node with matching data in the subtree.
     *
     *             Recursively searches through this node and all its descendants
     *             to find a node whose data matches the given node's data.
     *
     * @param[in]  node  The node whose data to search for.
     *
     * @return     Pointer to the found node, or nullptr if not found.
     */
    fb::mst<T>* search(const fb::mst<T>& node) const
    {
        if (this->compare(node.data))
            return const_cast<fb::mst<T>*>(this);

        for (auto i = this->_nodes.cbegin(); i != this->_nodes.cend(); i++)
        {
            auto found = (*i)->search(node);
            if (found != nullptr)
                return found;
        }

        return nullptr;
    }

public:
    /**
     * @brief      Gets an iterator to the beginning of the child nodes.
     *
     * @return     Iterator to the first child node.
     */
    node_list::iterator begin()
    {
        return _nodes.begin();
    }

public:
    /**
     * @brief      Gets an iterator to the end of the child nodes.
     *
     * @return     Iterator to one past the last child node.
     */
    node_list::iterator end()
    {
        return _nodes.end();
    }

public:
    /**
     * @brief      Gets a const iterator to the beginning of the child nodes.
     *
     * @return     Const iterator to the first child node.
     */
    node_list::const_iterator begin() const
    {
        return _nodes.cbegin();
    }

public:
    /**
     * @brief      Gets a const iterator to the end of the child nodes.
     *
     * @return     Const iterator to one past the last child node.
     */
    node_list::const_iterator end() const
    {
        return _nodes.cend();
    }

public:
    /**
     * @brief      Pure virtual function for comparing node data.
     *
     *             Derived classes must implement this function to define
     *             how node data should be compared for searching and matching.
     *
     * @param[in]  data  The data to compare against this node's data.
     *
     * @return     True if the data matches, false otherwise.
     */
    virtual bool compare(const T& data) const = 0;
};

} // namespace fb

#endif