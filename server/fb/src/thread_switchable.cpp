#include <fb/thread_switchable.h>
#include <fb/thread.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::thread_switchable, "fb.thread.switchable")
END_LUA_EXTENSION; // clang-format on

fb::thread_switchable::thread_switchable(uint32_t id) :
    fb::lua::luable(id)
{ }

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

bool fb::thread_switchable::matched_thread() const
{
    return std::this_thread::get_id() == this->thread()->id();
}