#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <fb/dead_lock.h>

namespace fb {

class lock_error : public std::runtime_error
{
public:
    lock_error() : std::runtime_error("lock failed")
    {}
    ~lock_error() = default;
};

class concurrent
{
protected:
    fb::dead_lock_detector            root;

protected:
    concurrent()
    {}
    ~concurrent()
    {}

protected:
    void add(fb::dead_lock_detector& node)
    {
        if (node.parent == nullptr)
            this->root.add(node);
    }

    void assert_dead_lock(const fb::dead_lock_detector& node)
    {
        auto& root = static_cast<const dead_lock_detector&>(node.root());
        root.assert_circulated_lock();
        this->root.assert_dead_lock(root);
    }
};

}

#endif