#include <fb/core/mst.h>

fb::mst::mst()
{}

fb::mst::mst(const std::string& key, const std::shared_ptr<mst>& parent) : key(key), parent(parent)
{}

fb::mst::~mst()
{}

std::string fb::mst::keys(const fb::mst::node_route& nodes)
{
    auto sstream = std::stringstream();
    auto i = nodes.cbegin();
    if(i != nodes.cend())
        sstream << (*i)->key;

    while (++i != nodes.cend())
    {
        sstream << '-' << (*i)->key;
    }

    return sstream.str();
}

bool fb::mst::contains(const fb::mst* node) const
{
    if (this->key != node->key)
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

bool fb::mst::subtree(const fb::mst* sub) const
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

fb::mst* fb::mst::search(const fb::mst* node) const
{
    if (this->key == node->key)
        return const_cast<fb::mst*>(this);

    for (auto i = this->_nodes.cbegin(); i != this->_nodes.cend(); i++)
    {
        auto found = (*i)->search(node);
        if (found != nullptr)
            return found;
    }

    return nullptr;
}

void fb::mst::travel(const std::function<bool(const node_route&)>& fn) const
{
    auto queue = std::queue<fb::mst::node_route>();
    if (this->key.empty())
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
        auto nodes = fb::mst::node_route(queue.front());
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
                auto route = fb::mst::node_route(nodes);
                route.push_back(child.get());
                queue.push(route);
            }
        }
    }
}

void fb::mst::add(const std::shared_ptr<fb::mst>& node)
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

const fb::mst* fb::mst::root() const
{
    const mst* node = this;
    while (node->parent != nullptr)
    {
        node = node->parent.get();
    }

    return node;
}

void fb::mst::assert_circulated_lock() const
{
    auto errors = std::vector<std::string>();

    this->travel([this, &errors](auto& route)
    {
        for(auto i1 = route.cbegin(), last = std::prev(route.cend()); i1 != last; i1++)
        {
            for(auto i2 = std::next(i1); i2 != route.cend(); i2++)
            {
                auto begin = *i1;
                auto end = *i2;

                if(begin->key == end->key)
                {
                    auto sstream = std::stringstream();
                    for (auto n : route)
                    {
                        if (n == begin || n == end)
                            sstream << '[' << n->key << ']';
                        else
                            sstream << n->key;

                        if (n != *last)
                            sstream << '-';
                    }
                    errors.push_back(sstream.str());
                }
            }
        }

        return false;
    });

    if (errors.size() > 0)
    {
        auto sstream = std::stringstream();
        sstream << "Circulated lock detected : " << boost::algorithm::join(errors, ", ");
        throw std::runtime_error(sstream.str());
    }
}

void fb::mst::assert_dead_lock(const fb::mst* node) const
{
    auto errors = std::vector<std::string>();
    node->travel([this, &errors](const mst::node_route& route)
    {
        for(auto i1 = route.cbegin(), last = std::prev(route.cend()); i1 != last; i1++)
        {
            for(auto i2 = std::next(i1); i2 != route.cend(); i2++)
            {
                auto begin = *i1;
                auto end = *i2;

                auto found = this->search(end);
                if (found != nullptr && found->search(begin) != nullptr)
                {
                    auto sstream = std::stringstream();
                    for (auto n : route)
                    {
                        if (n == begin || n == end)
                            sstream << '[' << n->key << ']';
                        else
                            sstream << n->key;

                        if (n != *last)
                            sstream << '-';
                    }
                    errors.push_back(sstream.str());
                }
            }
        }

        return false;
    });

     if (errors.size() > 0)
     {
         auto sstream = std::stringstream();
         sstream << "DeadLock detected : " << boost::algorithm::join(errors, ", ");
         throw std::runtime_error(sstream.str());
     }
}