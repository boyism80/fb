#include <fb/core/mst.h>

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

void fb::mst::assert_circulated_route(const node_route& route) const
{
    auto keys = std::set<std::string>();
    for (auto node : route)
    {
        if (keys.contains(node->key))
            throw std::runtime_error("Circulated lock detected : " + this->keys(route));

        keys.insert(node->key);
    }
}

void fb::mst::assert_dead_lock(const node_route& route1, const node_route& route2) const
{
    auto keys1 = keys(route1);
    auto keys2 = keys(route2);

    for (auto i1 = route1.cbegin(); i1 != --route1.cend(); i1++)
    {
        for(auto i2 = i1; i2 != route1.cend(); i2++)
        {
            auto& begin = (*i1)->key;
            auto& end = (*i2)->key;

            int n = 0, pos1 = -1, pos2 = -1;
            for (auto i3 = route2.cbegin(); i3 != route2.cend(); i3++, n++)
            {
                if (pos1 != -1 && (*i3)->key == begin)
                    pos1 = n;
                else if (pos2 != -1 && (*i3)->key == end)
                    pos2 = n;

                if (pos1 != -1 && pos2 != -1 && pos1 > pos2)
                {
                    auto sstream = std::stringstream();
                    sstream << "DeadLock detected : "
                        << keys(route1)
                        << "<=>"
                        << keys(route2);

                    throw std::runtime_error(sstream.str());
                }
            }
        }
    }
}

fb::mst* fb::mst::add(const mst& node)
{
    return this->add(node.key, node.parent);
}

fb::mst* fb::mst::add(const std::string& key, const fb::mst* parent)
{
    auto ptr = new fb::mst(key, parent);
    this->_nodes.push_back(std::unique_ptr<fb::mst>(ptr));
    return ptr;
}

const fb::mst* fb::mst::root()
{
    const mst* node = this;
    while (node->parent != nullptr)
    {
        node = node->parent;
    }

    return node;
}

const fb::mst::node_list& fb::mst::nodes() const
{
    return this->_nodes;
}

std::vector<fb::mst::node_route> fb::mst::routes() const
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

    auto visit = std::set<std::string>();
    auto result = std::vector<fb::mst::node_route>();
    while (queue.empty() == false)
    {
        auto node = fb::mst::node_route(queue.front());
        queue.pop();

        auto last = node.back();
        if (last->_nodes.empty())
        {
            auto keys = this->keys(node);
            if (visit.contains(keys))
                continue;

            visit.insert(key);
            result.push_back(node);
        }
        else
        {
            for (auto& child : last->_nodes)
            {
                auto route = fb::mst::node_route(node);
                route.push_back(child.get());
                queue.push(route);
            }
        }
    }

    return result;
}

void fb::mst::assert_dead_lock() const
{
    auto routes = this->routes();
    auto errors = std::vector<std::string>();

    for (auto& route : routes)
    {
        try
        {
            this->assert_circulated_route(route);
        }
        catch(std::exception& e)
        { 
            errors.push_back(e.what());
        }
    }

    for (int i1 = 0; i1 < routes.size() - 1; i1++)
    {
        for (int i2 = i1 + 1; i2 < routes.size(); i2++)
        {
            try
            {
                this->assert_dead_lock(routes[i1], routes[i2]);
            }
            catch (std::exception& e)
            {
                errors.push_back(e.what());
            }
        }
    }

    if (errors.size() > 0)
    {
        throw std::runtime_error(boost::algorithm::join(errors, "\n"));
    }
}