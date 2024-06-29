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
    using node_list = std::vector<mst<T>*>;
    using node_route = std::list<std::reference_wrapper<const mst<T>>>;
    using init_fn = std::function<const T&()>;

private:
    std::vector<std::unique_ptr<fb::mst<T>>> _allocated;
    node_list                   _nodes;

public:
    const T                     data;
    const fb::mst<T>*           parent = nullptr;

protected:
    mst(const mst<T>&) = delete;
    mst(const T& data, const mst<T>* parent);

public:
    virtual ~mst() = default;

protected:
    bool                        contains(const fb::mst<T>& node) const;
    bool                        subtree(const fb::mst<T>& sub) const;
    void                        travel(const std::function<bool(const node_route&)>& fn) const;

public:
    template <typename R, typename... Args>
    R&                          add(Args&&... args);
    template <typename R>
    R&                          add(R& node);
    const mst<T>&               root() const;
    fb::mst<T>*                 search(const fb::mst<T>& node) const;

public:
    node_list::iterator         begin();
    node_list::iterator         end();
    node_list::const_iterator   begin() const;
    node_list::const_iterator   end() const;
    virtual bool                compare(const T&) const = 0;
};

}

template <typename T>
fb::mst<T>::mst(const T& data, const mst<T>* parent) : data(data), parent(parent)
{ }

template <typename T>
bool fb::mst<T>::contains(const fb::mst<T>& node) const
{
    if (this->data != node.data)
        return false;

    if(this->_nodes.size() < node._nodes.size())
        return false;

    if (node._nodes.size() == 0)
        return true;

    for(auto i1 = this->_nodes.cbegin(); i1 != this->_nodes.cend(); i1++)
    {
        auto match = true;
        for(auto i2 = node._nodes.cbegin(); i2 != node._nodes.cend(); i2++)
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
bool fb::mst<T>::subtree(const fb::mst<T>& sub) const
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
            queue.push({ *node });
    }
    else
    {
        queue.push({ *this });
    }

    while (queue.empty() == false)
    {
        auto nodes = fb::mst<T>::node_route(queue.front());
        queue.pop();

        auto& back = nodes.back().get();
        if (back._nodes.empty())
        {
            if(fn(nodes) == true)
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

template <typename T>
template <typename R, typename... Args>
R& fb::mst<T>::add(Args&&... args)
{
    auto ptr = std::unique_ptr<fb::mst<T>>(new R(std::forward<Args>(args)...));
    auto node = ptr.get();
    this->_allocated.push_back(std::move(ptr));

    this->add(*node);
    return static_cast<R&>(*node);
}

template <typename T>
template <typename R>
R& fb::mst<T>::add(R& node)
{
    this->_nodes.push_back(&node);
    return static_cast<R&>(*this);
}

template <typename T>
const fb::mst<T>& fb::mst<T>::root() const
{
    const mst* node = this;
    while (node->parent != nullptr)
    {
        node = node->parent;
    }

    return *node;
}

template <typename T>
fb::mst<T>* fb::mst<T>::search(const fb::mst<T>& node) const
{
    if(this->compare(node.data))
        return const_cast<fb::mst<T>*>(this);

    for (auto i = this->_nodes.cbegin(); i != this->_nodes.cend(); i++)
    {
        auto found = (*i)->search(node);
        if (found != nullptr)
            return found;
    }

    return nullptr;
}

template <typename T>
fb::mst<T>::node_list::iterator fb::mst<T>::begin()
{
    return _nodes.begin();
}

template <typename T>
fb::mst<T>::node_list::iterator fb::mst<T>::end()
{
    return _nodes.end();
}

template <typename T>
fb::mst<T>::node_list::const_iterator fb::mst<T>::begin() const
{
    return _nodes.cbegin();
}

template <typename T>
fb::mst<T>::node_list::const_iterator fb::mst<T>::end() const
{
    return _nodes.cend();
}

#endif