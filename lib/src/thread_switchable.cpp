#include <fb/thread_switchable.h>
#include <fb/thread.h>

void fb::thread_switchable::assert_thread() const
{
#if defined DEBUG | defined _DEBUG
    auto thread = this->thread();
    if (thread == nullptr)
        throw std::runtime_error("active thread is null");

    if (std::this_thread::get_id() != thread->id())
        throw std::runtime_error("active thread not matched");
#endif
}