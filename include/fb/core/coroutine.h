#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <exception>
#include <optional>
#include <coroutine>

namespace fb {

template <typename R>
class awaitable;

template <typename R>
using awaitable_suspend_handler = std::function<void(awaitable<R>&)>;

template <typename R>
class awaitable
{
private:
    const awaitable_suspend_handler<R> _on_suspend;

public:
    R*                              result;
    std::optional<std::exception>   e;
    std::coroutine_handle<>         handler;

public:
    awaitable(const awaitable_suspend_handler<R>& on_suspend) : _on_suspend(on_suspend), result(nullptr)
    { }
    virtual ~awaitable()
    { }

    virtual bool                    await_ready()
    {
        return false;
    }
    virtual void                    await_suspend(std::coroutine_handle<> h)
    {
        this->handler = h;
        this->_on_suspend(*this);
    }
    virtual R                       await_resume()
    {
        if (this->e.has_value())
            throw this->e.value();

        return std::move(*this->result);
    }
};

}

#endif