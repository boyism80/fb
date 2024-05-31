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

namespace fb {

class mst
{
public:
    using node_list = std::vector<std::unique_ptr<mst>>;
    using node_route = std::list<const mst*>;

private:
    node_list _nodes;

public:
    const std::string key;
    const mst* parent;

public:
    mst(const std::string& key, mst* parent = nullptr) : key(key), parent(parent)
    {}
    mst(const mst& node) : key(node.key), parent(node.parent)
    { }
    ~mst() = default;

private:
    static std::string      keys(const node_route& nodes);
    void                    assert_circulated_route(const node_route& route) const;
    void                    assert_dead_lock(const node_route& route1, const node_route& route2) const;

public:
    void                    add(mst* node);
    const mst*              root();
    const node_list&        nodes() const;
    std::vector<node_route> routes() const;
    void                    assert_dead_lock() const;
};

}

#endif