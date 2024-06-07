#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <exception>
#include <optional>
#include <coroutine>

namespace fb {

#pragma region abstract
template <typename E>
class awaitable
{
public:
    std::unique_ptr<E> error;
    std::coroutine_handle<> handler;

public:
    awaitable() = default;
    ~awaitable() = default;

public:
    virtual bool await_ready()
    {
        return false;
    }

    virtual void await_suspend(std::coroutine_handle<> h)
    {
        this->handler = h;
    }

    void await_resume()
    {
        if (this->error != nullptr)
            throw* this->error;
    }
};

template <typename T, typename E>
class awaiter;

template <typename T, typename E>
using awaiter_handler = std::function<void(awaiter<T, E>&)>;



template <typename T, typename E = std::runtime_error>
class base_awaiter : public awaitable<E>
{
private:
    const awaiter_handler<T, E> _callback;

protected:
    base_awaiter(const awaiter_handler<T, E>& callback) : _callback(callback)
    {}

public:
    virtual ~base_awaiter()
    {}

public:
    virtual void await_suspend(std::coroutine_handle<> h)
    {
        awaitable<E>::await_suspend(h);
        this->_callback(static_cast<awaiter<T, E>&>(*this));
    }
};
#pragma endregion

template <typename T, typename E = std::runtime_error>
class awaiter : public base_awaiter<T, E>
{
public:
    T* result;

public:
    awaiter(const awaiter_handler<T, E>& callback) : base_awaiter<T, E>(callback), result(nullptr)
    { }
    virtual ~awaiter()
    { }

public:
    T await_resume()
    {
        base_awaiter<T, E>::await_resume();
        return std::move(*this->result);
    }
};

template <typename E>
class awaiter<void, E> : public base_awaiter<void, E>
{
public:
    awaiter(const awaiter_handler<void, E>& callback) : base_awaiter<void, E>(callback)
    { }
    virtual ~awaiter()
    { }
};


template <typename T>
class task : public awaitable<std::runtime_error>
{
public:
    class promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    class promise_type
    {
    public:
        std::optional<T> _value;
        std::exception_ptr _exception;
        awaitable<std::runtime_error>* _awaitable = nullptr;

        auto get_return_object() { return task(handle_type::from_promise(*this)); }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception()
        {
            this->_exception = std::current_exception();
            if (_awaitable != nullptr)
                _awaitable->handler.resume();
        }

        template<std::convertible_to<T> From>
        void return_value(From&& from)
        {
            this->_value = std::forward<From>(from);
            if (_awaitable != nullptr)
                _awaitable->handler.resume();
        }
    };

    handle_type _handler;

    task(handle_type handler) : _handler(handler) { }
    ~task() { }

    void await_suspend(std::coroutine_handle<> h)
    {
        awaitable<std::runtime_error>::await_suspend(h);

        auto& promise = this->_handler.promise();
        promise._awaitable = this;
        if (promise._exception)
            std::rethrow_exception(this->_handler.promise()._exception);

        if (promise._value.has_value())
        {
            awaitable<std::runtime_error>::handler.resume();
        }
    }

    T await_resume()
    {
        awaitable<std::runtime_error>::await_resume();
        return this->value();
    }

    bool done()
    {
        return this->_handler.done();
    }

    void operator()()
    {
        auto& promise = this->_handler.promise();
        if (promise._awaitable != nullptr)
        {
            promise._awaitable->_handler.resume();
        }
        else
        {
            this->_handler.resume();
        }

        if (promise._exception)
            std::rethrow_exception(promise._exception);
    }

    T value()
    {
        auto& promise = this->_handler.promise();
        if (promise._exception)
            std::rethrow_exception(promise._exception);

        if (promise._value.has_value() == false)
            throw std::runtime_error("value is empty");

        return *promise._value;
    }
};

template <>
class task<void> : public awaitable<std::runtime_error>
{
public:
    class promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    class promise_type
    {
    public:
        std::exception_ptr _exception;
        awaitable<std::runtime_error>* _awaitable = nullptr;

        auto get_return_object() { return task(handle_type::from_promise(*this)); }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() 
        {
            this->_exception = std::current_exception(); 
            if (_awaitable != nullptr)
                _awaitable->handler.resume();
        }
        void return_void() 
        {
            if (_awaitable != nullptr)
                _awaitable->handler.resume();
        }
    };

    handle_type _handler;

    task(handle_type handler) : _handler(handler) { }
    ~task() { }

    void await_suspend(std::coroutine_handle<> h)
    {
        awaitable<std::runtime_error>::await_suspend(h);

        auto& promise = this->_handler.promise();
        promise._awaitable = this;
        if (promise._exception)
            std::rethrow_exception(this->_handler.promise()._exception);
    }

    void await_resume()
    {
        awaitable<std::runtime_error>::await_resume();
    }

    bool done()
    {
        return this->_handler.done();
    }

    void operator()()
    {
        auto& promise = this->_handler.promise();
        if (promise._awaitable != nullptr)
        {
            promise._awaitable->handler.resume();
        }
        else
        {
            this->_handler.resume();
        }

        if (promise._exception)
            std::rethrow_exception(promise._exception);
    }
};

}

#endif