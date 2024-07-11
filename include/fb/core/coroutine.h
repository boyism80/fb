#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <exception>
#include <stdexcept>
#include <optional>
#include <coroutine>
#include <functional>
#include <chrono>

using namespace std::chrono_literals;

namespace fb {

#pragma region abstract
class awaitable
{
public:
    std::exception_ptr      error = nullptr;
    std::coroutine_handle<> handler;

public:
    awaitable() = default;
    awaitable(const awaitable&) = delete;
    awaitable(awaitable&& r) noexcept : handler(r.handler), error(r.error)
    {
        r.handler = nullptr;
        r.error = nullptr;
    }
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

    void operator = (const awaitable&) = delete;
    void operator = (awaitable&& r) noexcept
    {
        this->handler = r.handler;
        r.handler = nullptr;
        
        this->error = r.error;
        r.error = nullptr;
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
    base_awaiter(const base_awaiter&) = delete;
    base_awaiter(base_awaiter&& r) : awaitable(static_cast<base_awaiter&&>(r)), _callback(r._callback)
    {
        
    }

public:
    virtual ~base_awaiter()
    {}

public:
    virtual void await_suspend(std::coroutine_handle<> h)
    {
        awaitable::await_suspend(h);
        this->_callback(static_cast<awaiter<T>&>(*this));
    }

    void operator = (const base_awaiter&) = delete;
    void operator = (base_awaiter&& r)
    {
        awaitable::operator = (static_cast<awaitable&&>(r));
    }
};
#pragma endregion

template <typename T, typename T2 = void> struct awaiter_result; // Use this one instead.

template <typename T>
struct awaiter_result<T, typename std::enable_if<std::is_class<T>::value>::type>
{
    typedef std::optional<std::reference_wrapper<T>> type;
};

template <typename T>
struct awaiter_result<T, typename std::enable_if<!std::is_class<T>::value>::type>
{
    typedef T type;
};

template <typename T = void>
class awaiter : public base_awaiter<T>
{
public:
    typename awaiter_result<T>::type result;

public:
    awaiter(const awaiter_handler<T>& callback) : base_awaiter<T>(callback)
    { }
    virtual ~awaiter()
    { }

public:
    T& await_resume()
    {
        base_awaiter<T>::await_resume();
        if constexpr (std::is_class_v<T>)
        {
            return this->result.value().get();
        }
        else
        {
            return result;
        }
    }
};

template <>
class awaiter<void> : public base_awaiter<void>
{
public:
    awaiter(const awaiter_handler<void>& callback) : base_awaiter<void>(callback)
    { }
    awaiter(const awaiter&) = delete;
    awaiter(awaiter&& r) : base_awaiter(static_cast<base_awaiter&&>(r))
    { }
    virtual ~awaiter()
    { }

public:
    void operator = (const awaiter&) = delete;
    void operator = (awaiter&& r)
    {
        base_awaiter<void>::operator=(static_cast<base_awaiter<void>&&>(r));
    }
};


template <typename T, typename T2 = void> struct task_result; // Use this one instead.

template <typename T>
struct task_result<T, typename std::enable_if<std::is_class<T>::value>::type>
{
    typedef std::optional<std::reference_wrapper<T>> type;
};

template <typename T>
struct task_result<T, typename std::enable_if<!std::is_class<T>::value>::type>
{
    typedef std::optional<T> type;
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
        task_result<T>::type _value;
        std::exception_ptr error = nullptr;
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
    task(const task&) = delete;
    task(task&& r) : awaitable(static_cast<awaitable&&>(r)), _handler(r._handler)
    {
        r._handler = nullptr;
    }
    ~task()
    {
        if(this->_handler)
            this->_handler.destroy();
    }

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

    T& await_resume()
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

    void operator = (const task&) = delete;
    void operator=  (task&& r) noexcept
    {
        awaitable::operator = (static_cast<awaitable&&>(r));
        this->_handler = r._handler;
        r._handler = nullptr;
    }

    T& value()
    {
        auto& promise = this->_handler.promise();
        if (promise.error)
            std::rethrow_exception(promise.error);

        if (promise._value.has_value() == false)
            throw std::runtime_error("value is empty");

        if constexpr (std::is_class_v<T>)
        {
            return promise._value.value().get();
        }
        else
        {
            return promise._value.value();
        }
    }

    void wait()
    {
        constexpr auto term = 100ms;
        while(!this->done())
        {
            std::this_thread::sleep_for(term);
        }
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
        std::exception_ptr error = nullptr;
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
    task(const task&) = delete;
    task(task&& r) noexcept : awaitable(static_cast<awaitable&&>(r)), _handler(r._handler)
    {
        r._handler = nullptr;
    }
    ~task()
    {
        if(this->_handler)
            this->_handler.destroy();
    }

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

    void operator = (const task&) = delete;
    void operator = (task&& r) noexcept
    {
        awaitable::operator = (static_cast<awaitable&&>(r));
        this->_handler = r._handler;
        r._handler = nullptr;
    }

    void wait()
    {
        constexpr auto term = 100ms;
        while(!this->done())
        {
            std::this_thread::sleep_for(term);
        }
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