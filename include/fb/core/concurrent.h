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
    std::shared_ptr<fb::dead_lock_detector>            _root;

protected:
    concurrent() : _root(std::make_shared<fb::dead_lock_detector>())
    {}
    ~concurrent()
    {}

protected:
    void add(std::shared_ptr<fb::dead_lock_detector>& node)
    {
        if (node->parent == nullptr)
            this->_root->add(node);
    }

    std::shared_ptr<fb::dead_lock_detector> alloc(const std::string& key, std::shared_ptr<fb::dead_lock_detector>& trans)
    {
        auto current = std::make_shared<fb::dead_lock_detector>(key, trans);
        if(trans != nullptr)
            trans->add(current);

        return current;
    }

    void assert_dead_lock(std::shared_ptr<fb::dead_lock_detector>& node)
    {
        auto root = static_cast<const dead_lock_detector*>(node->root());
        root->assert_circulated_lock();
        this->_root->assert_dead_lock(root);
    }
};

}

#endif