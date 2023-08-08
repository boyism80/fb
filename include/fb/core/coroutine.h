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
    std::optional<std::string>      error;
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
        if (this->error.has_value())
            throw std::runtime_error(this->error.value());

        return std::move(*this->result);
    }
};

template <>
class awaitable<void>
{
private:
    const awaitable_suspend_handler<void> _on_suspend;

public:
    std::optional<std::string>      error;
    std::coroutine_handle<>         handler;

public:
    awaitable(const awaitable_suspend_handler<void>& on_suspend) : _on_suspend(on_suspend)
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
        if (this->error.has_value())
            throw std::runtime_error(this->error.value());
    }
};

}

#endif