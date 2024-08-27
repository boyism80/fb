#ifndef __PROMISE_H__
#define __PROMISE_H__

#include <coroutine>

namespace fb {

template <typename SUSPEND_TYPE>
class base_promise
{
public:
    std::exception_ptr          error = nullptr;
    std::coroutine_handle<>     parent;

    auto initial_suspend() { return SUSPEND_TYPE{}; }
    auto final_suspend() noexcept
    {
        return std::suspend_always{};
    }
    void unhandled_exception() { this->error = std::current_exception(); }
};

}

#endif // !__PROMISE_H__
