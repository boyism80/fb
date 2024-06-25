#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <fb/core/dead_lock.h>

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
private:
    fb::dead_lock_detector            _root;

protected:
    concurrent()
    {}
    ~concurrent()
    {}

protected:
    // void add(fb::dead_lock_detector* node)
    // {
    //     if (node->parent == nullptr)
    //         this->_root->add(node);
    // }

    fb::dead_lock_detector* alloc(const std::string& key, fb::dead_lock_detector* trans)
    {
        return trans->add<fb::dead_lock_detector>(key);
        // auto current = std::make_shared<fb::dead_lock_detector>(key, trans);
        // if(trans != nullptr)
        //     trans->add(current);

        // return current;
    }

    void assert_dead_lock(fb::dead_lock_detector* node)
    {
        auto root = static_cast<const dead_lock_detector*>(node->root());
        root->assert_circulated_lock();
        this->_root.assert_dead_lock(root);
    }
};

}

#endif