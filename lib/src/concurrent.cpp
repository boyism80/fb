#include <fb/concurrent.h>

using namespace fb;

lock_error::lock_error() : std::runtime_error("lock failed")
{}

void concurrent::add(fb::dead_lock_detector& node)
{
    if (node.parent == nullptr)
        this->root.add(node);
}

void concurrent::assert_dead_lock(const fb::dead_lock_detector& node)
{
    auto& root = static_cast<const dead_lock_detector&>(node.root());
    root.assert_circulated_lock();
    this->root.assert_dead_lock(root);
}