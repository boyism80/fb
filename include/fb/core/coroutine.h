#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <exception>
#include <stdexcept>
#include <optional>
#include <coroutine>
#include <functional>

namespace fb {

#pragma region abstract
class awaitable
{
public:
    std::exception_ptr      error;
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
        if (this->error)
            std::rethrow_exception(this->error);
    }
};

template <typename T>
class awaiter;

template <typename T>
using awaiter_handler = std::function<void(awaiter<T>&)>;



template <typename T>
class base_awaiter : public awaitable
{
private:
    const awaiter_handler<T> _callback;

protected:
    base_awaiter(const awaiter_handler<T>& callback) : _callback(callback)
    {}

public:
    virtual ~base_awaiter()
    {}

public:
    virtual void await_suspend(std::coroutine_handle<> h)
    {
        awaitable::await_suspend(h);
        this->_callback(static_cast<awaiter<T>&>(*this));
    }
};
#pragma endregion

template <typename T = void>
class awaiter : public base_awaiter<T>
{
public:
    T* result;

public:
    awaiter(const awaiter_handler<T>& callback) : base_awaiter<T>(callback), result(nullptr)
    { }
    virtual ~awaiter()
    { }

public:
    T await_resume()
    {
        base_awaiter<T>::await_resume();
        return std::move(*this->result);
    }
};

template <>
class awaiter<void> : public base_awaiter<void>
{
public:
    awaiter(const awaiter_handler<void>& callback) : base_awaiter<void>(callback)
    { }
    virtual ~awaiter()
    { }
};


template <typename T = void>
class task : public awaitable
{
public:
    class promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    class promise_type
    {
    public:
        std::optional<T> _value;
        std::exception_ptr error;
        awaitable* _awaitable = nullptr;

        auto get_return_object() { return task(handle_type::from_promise(*this)); }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception()
        {
            this->error = std::current_exception();
            if (this->_awaitable != nullptr)
                this->_awaitable->handler.resume();
        }

        template<std::convertible_to<T> From>
        void return_value(From&& from)
        {
            this->_value = std::forward<From>(from);
            if (this->_awaitable != nullptr)
                this->_awaitable->handler.resume();
        }
    };

    handle_type _handler;

    task(handle_type handler) : _handler(handler) { }
    ~task() { }

    void await_suspend(std::coroutine_handle<> h)
    {
        awaitable::await_suspend(h);

        auto& promise = this->_handler.promise();
        promise._awaitable = this;
        if (promise.error)
            std::rethrow_exception(this->_handler.promise().error);

        if (promise._value.has_value())
        {
            awaitable::handler.resume();
        }
    }

    T await_resume()
    {
        awaitable::await_resume();
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

        if (promise.error)
            std::rethrow_exception(promise.error);
    }

    T value()
    {
        auto& promise = this->_handler.promise();
        if (promise.error)
            std::rethrow_exception(promise.error);

        if (promise._value.has_value() == false)
            throw std::runtime_error("value is empty");

        return *promise._value;
    }
};

template <>
class task<void> : public awaitable
{
public:
    class promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    class promise_type
    {
    public:
        std::exception_ptr error;
        awaitable* _awaitable = nullptr;

        auto get_return_object() { return task(handle_type::from_promise(*this)); }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() 
        {
            this->error = std::current_exception(); 
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
        awaitable::await_suspend(h);

        auto& promise = this->_handler.promise();
        promise._awaitable = this;
        if (promise.error)
            std::rethrow_exception(this->_handler.promise().error);
    }

    void await_resume()
    {
        awaitable::await_resume();
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

        if (promise.error)
            std::rethrow_exception(promise.error);
    }
};

template <typename OUTPUT, typename INPUT = void>
struct generator
{
    struct input {};

    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct passthru_value
    {
        INPUT& ret_;

        bool await_ready() { return true; }
        void await_suspend(coro_handle) {}
        INPUT& await_resume() { return ret_; }
    };

    struct promise_type {
        std::optional<OUTPUT> current_value;
        INPUT input_value;


        auto get_return_object() { return generator{ coro_handle::from_promise(*this) }; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        auto yield_value(OUTPUT value)
        {
            current_value = value;
            return std::suspend_always{};
        }

        void return_void() {}

        auto await_transform(input)
        {
            return passthru_value{ input_value };
        }
    };

    bool next() { return coro ? (coro.resume(), !coro.done()) : false; }
    OUTPUT value()
    {
        auto& promise = coro.promise();
        if (promise.has_value() == false)
            throw std::runtime_error("has no value");

        return promise.current_value.value();
    }

    void send(const INPUT& input)
    {
        coro.promise().input_value = input;
    }

    void send(INPUT&& input)
    {
        coro.promise().input_value = std::move(input);
    }

    generator(generator const& rhs) = delete;
    generator(generator&& rhs)
        :coro(rhs.coro)
    {
        rhs.coro = nullptr;
    }
    ~generator()
    {
        if (coro)
            coro.destroy();
    }
private:
    generator(coro_handle h) : coro(h) {}
    coro_handle coro;
};

template <typename OUTPUT>
struct generator<OUTPUT, void>
{
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        std::optional<OUTPUT> current_value;
        auto get_return_object() { return generator{ coro_handle::from_promise(*this) }; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        auto yield_value(OUTPUT value)
        {
            current_value = value;
            return std::suspend_always{};
        }

        void return_void() {}
    };

    bool next() { return coro ? (coro.resume(), !coro.done()) : false; }
    OUTPUT value()
    {
        auto& promise = coro.promise();
        if (promise.current_value.has_value() == false)
            throw std::runtime_error("has no value");

        return promise.current_value.value();
    }

    generator(generator const& rhs) = delete;
    generator(generator&& rhs) :coro(rhs.coro)
    {
        rhs.coro = nullptr;
    }
    ~generator()
    {
        if (coro)
            coro.destroy();
    }
private:
    generator(coro_handle h) : coro(h) {}
    coro_handle coro;
};

}

#endif