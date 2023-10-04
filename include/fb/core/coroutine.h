#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <exception>
#include <optional>
#include <coroutine>

namespace fb {

struct task
{
    struct promise_type
    {
        auto get_return_object() { return task{}; }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

template <typename R, typename E>
class awaitable;

template <typename R, typename E>
using awaitable_suspend_handler = std::function<void(awaitable<R, E>&)>;

template <typename R, typename E = std::runtime_error>
class awaitable
{
private:
    const awaitable_suspend_handler<R, E> _on_suspend;

public:
    R*                              result;
    std::unique_ptr<E>              error;
    std::coroutine_handle<>         handler;

public:
    awaitable(const awaitable_suspend_handler<R, E>& on_suspend) : _on_suspend(on_suspend), result(nullptr)
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
        if (this->error != nullptr)
            throw *this->error;

        return std::move(*this->result);
    }
};

template <typename E>
class awaitable<void, E>
{
private:
    const awaitable_suspend_handler<void, E> _on_suspend;

public:
    std::unique_ptr<E>              error;
    std::coroutine_handle<>         handler;

public:
    awaitable(const awaitable_suspend_handler<void, E>& on_suspend) : _on_suspend(on_suspend)
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
    virtual void                    await_resume()
    {
        if (this->error != nullptr)
            throw *this->error;
    }
};

}

#endif