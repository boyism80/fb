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

class mst
{
public:
    using node_list = std::vector<std::shared_ptr<mst>>;
    using node_route = std::list<const mst*>;

private:
    node_list                   _nodes;

public:
    const std::string           key;
    const std::shared_ptr<mst>  parent = nullptr;

public:
    mst();
    mst(const std::string& key, const std::shared_ptr<mst>& parent);
    ~mst();

private:
    bool                        contains(const fb::mst* node) const;
    bool                        subtree(const fb::mst* sub) const;
    fb::mst*                    search(const fb::mst* node) const;
    void                        travel(const std::function<bool(const node_route&)>& fn) const;

public:
    void                        add(const std::shared_ptr<fb::mst>& node);
    const mst*                  root() const;
    void                        assert_circulated_lock() const;
    void                        assert_dead_lock(const fb::mst* node) const;

public:
    static std::string          keys(const node_route& nodes);
};

}

#endif