#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <exception>
#include <optional>
#include <coroutine>

namespace fb {

template <typename T>
struct task
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        std::optional<T> _value;
        std::exception_ptr _exception;
        std::coroutine_handle<> _handler;

        auto get_return_object()        { return task(handle_type::from_promise(*this)); }
        auto initial_suspend()          { return std::suspend_never{}; }
        auto final_suspend() noexcept   { return std::suspend_always{}; }
        void unhandled_exception()      { this->_exception = std::current_exception(); }

        template<std::convertible_to<T> From>
        void return_value(From&& from)  { this->_value = std::forward<From>(from); if(_handler) this->_handler.resume(); }
    };

    handle_type _handler;

    task(handle_type handler) : _handler(handler) { }
    ~task() { }

    bool await_ready()
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h)
    {
        this->_handler.promise()._handler = h;

        if (this->_handler.promise()._exception)
            std::rethrow_exception(this->_handler.promise()._exception);
    }

    T await_resume()
    {
        return this->value();
    }

    bool done()
    {
        return this->_handler.done();
    }

    void operator()()
    {
        this->_handler();
        if (this->_handler.promise()._exception)
            std::rethrow_exception(this->_handler.promise()._exception);
    }

    T value()
    {
        if (this->_handler.promise()._exception)
            std::rethrow_exception(this->_handler.promise()._exception);

        if (this->_handler.promise()._value.has_value() == false)
            throw std::runtime_error("value is empty");

        return *this->_handler.promise()._value;
    }
};

template <>
struct task<void>
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        std::exception_ptr _exception;
        std::coroutine_handle<> _handler;

        auto get_return_object()        { return task(handle_type::from_promise(*this)); }
        auto initial_suspend()          { return std::suspend_never{}; }
        auto final_suspend() noexcept   { return std::suspend_always{}; }
        void return_void()              { }
        void unhandled_exception()      { this->_exception = std::current_exception(); if(this->_handler) this->_handler.resume(); }
    };

    handle_type _handler;

    task(handle_type handler) : _handler(handler) {}
    ~task() { }

    bool await_ready()
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h)
    {
        this->_handler.promise()._handler = h;

        if (this->_handler.promise()._exception)
            std::rethrow_exception(this->_handler.promise()._exception);
    }

    void await_resume()
    {

    }

    bool done()
    {
        return this->_handler.done();
    }

    void operator()()
    {
        this->_handler();
        if (this->_handler.promise()._exception)
            std::rethrow_exception(this->_handler.promise()._exception);
    }
};

template <typename R, typename E>
class awaitable;

template <typename R, typename E>
using awaitable_callback_handler = std::function<void(awaitable<R, E>&)>;

template <typename R, typename E = std::runtime_error>
class awaitable
{
private:
    const awaitable_callback_handler<R, E> _callback;

public:
    R*                              result;
    std::unique_ptr<E>              error;
    std::coroutine_handle<>         handler;

public:
    awaitable(const awaitable_callback_handler<R, E>& callback) : _callback(callback), result(nullptr)
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
        this->_callback(*this);
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
    const awaitable_callback_handler<void, E> _callback;

public:
    std::unique_ptr<E>              error;
    std::coroutine_handle<>         handler;

public:
    awaitable(const awaitable_callback_handler<void, E>& callback) : _callback(callback)
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
        this->_callback(*this);
    }
    virtual void                    await_resume()
    {
        if (this->error != nullptr)
            throw *this->error;
    }
};

}

#endif