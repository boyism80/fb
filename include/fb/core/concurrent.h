#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <fb/core/mst.h>

namespace fb {

class lock_error : public std::exception
{
public:
    lock_error() : std::exception("lock failed")
    {}
    ~lock_error() = default;
};

class concurrent
{
private:
    std::shared_ptr<fb::mst>            _root;

protected:
    concurrent() : _root(std::make_shared<fb::mst>())
    {}
    ~concurrent()
    {}

protected:
    void add(std::shared_ptr<fb::mst>& node)
    {
        if (node->parent == nullptr)
            this->_root->add(node);
    }

    std::shared_ptr<fb::mst> alloc(const std::string& key, std::shared_ptr<fb::mst>& trans)
    {
        auto current = std::make_shared<fb::mst>(key, trans);
        if(trans != nullptr)
            trans->add(current);

        return current;
    }

    void assert_dead_lock(std::shared_ptr<fb::mst>& node)
    {
        auto root = node->root();
        root->assert_circulated_lock();
        this->_root->assert_dead_lock(root);
    }
};

}

#endif