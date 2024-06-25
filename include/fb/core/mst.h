#ifndef __MST_H__
#define __MST_H__

#include <vector>
#include <string>
#include <memory>
#include <queue>
#include <list>
#include <set>
#include <sstream>
#include <boost/algorithm/string/join.hpp>
#include <fb/core/logger.h>

namespace fb {

template <typename T>
class mst
{
public:
    using node_list = std::vector<std::shared_ptr<mst<T>>>;
    using node_route = std::list<const mst<T>*>;

private:
    node_list                   _nodes;

public:
    const T                     data;
    const std::shared_ptr<mst>  parent = nullptr;

protected:
    mst() = default;
    mst(const mst<T>&) = delete;
    mst(const T& data, const std::shared_ptr<mst<T>>& parent);

public:
    virtual ~mst() = default;

protected:
    bool                        contains(const fb::mst<T>* node) const;
    bool                        subtree(const fb::mst<T>* sub) const;
    void                        travel(const std::function<bool(const node_route&)>& fn) const;

public:
    void                        add(const std::shared_ptr<fb::mst<T>>& node);
    const mst<T>*               root() const;
    fb::mst<T>*                 search(const fb::mst<T>* node) const;
};

}

template <typename T>
fb::mst<T>::mst(const T& data, const std::shared_ptr<mst>& parent) : data(data), parent(parent)
{}

template <typename T>
bool fb::mst<T>::contains(const fb::mst<T>* node) const
{
    if (this->data != node->data)
        return false;

    if(this->_nodes.size() < node->_nodes.size())
        return false;

    if (node->_nodes.size() == 0)
        return true;

    for(auto i1 = this->_nodes.cbegin(); i1 != this->_nodes.cend(); i1++)
    {
        auto match = true;
        for(auto i2 = node->_nodes.cbegin(); i2 != node->_nodes.cend(); i2++)
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

template <typename T>
bool fb::mst<T>::subtree(const fb::mst<T>* sub) const
{
    if (this->contains(sub))
        return true;

    for(auto i1 = this->_nodes.cbegin(); i1 != this->_nodes.cend(); i1++)
    {
        if ((*i1)->subtree(sub))
            return true;
    }

    return false;
}

template <typename T>
void fb::mst<T>::travel(const std::function<bool(const node_route&)>& fn) const
{
    auto queue = std::queue<fb::mst<T>::node_route>();
    if (this->data.empty())
    {
        for (auto& node : this->_nodes)
            queue.push({ node.get() });
    }
    else
    {
        queue.push({ this });
    }

    while (queue.empty() == false)
    {
        auto nodes = fb::mst<T>::node_route(queue.front());
        queue.pop();

        if (nodes.back()->_nodes.empty())
        {
            if(fn(nodes) == true)
                break;
        }
        else
        {
            for (auto& child : nodes.back()->_nodes)
            {
                auto route = fb::mst<T>::node_route(nodes);
                route.push_back(child.get());
                queue.push(route);
            }
        }
    }
}

template <typename T>
void fb::mst<T>::add(const std::shared_ptr<fb::mst<T>>& node)
{
    auto found = this->search(node.get());
    if (found == nullptr)
    {
        this->_nodes.push_back(node);
    }
    else
    {
        for (auto i = node->_nodes.cbegin(); i != node->_nodes.cend(); i++)
        {
            found->add(*i);
        }
    }
}

template <typename T>
const fb::mst<T>* fb::mst<T>::root() const
{
    const mst* node = this;
    while (node->parent != nullptr)
    {
        node = node->parent.get();
    }

    return node;
}

template <typename T>
fb::mst<T>* fb::mst<T>::search(const fb::mst<T>* node) const
{
    if (this->data == node->data)
        return const_cast<fb::mst<T>*>(this);

    for (auto i = this->_nodes.cbegin(); i != this->_nodes.cend(); i++)
    {
        auto found = (*i)->search(node);
        if (found != nullptr)
            return found;
    }

    return nullptr;
}

#endif