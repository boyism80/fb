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

template <typename T1, typename T2 = void> struct task_resume_type;

template <typename T>
struct task_resume_type<T, typename std::enable_if_t<std::is_void_v<T>>>
{
    typedef void type;
};

template <typename T>
struct task_resume_type<T, typename std::enable_if_t<!std::is_void_v<T>>>
{
    typedef T& type;
};

class base_promise;
using handle_type = std::coroutine_handle<base_promise>;

class base_promise
{
public:
    std::exception_ptr error = nullptr;
    handle_type parent, child;

    auto initial_suspend() { return std::suspend_never{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void unhandled_exception() { this->error = std::current_exception(); }
};

template <typename T> class task;
template <typename T> using awaiter_handler = std::function<void(task<T>&)>;

template <typename T>
class base_task
{
private:
    awaiter_handler<T> callback;

public:
    handle_type handler;

public:
    base_task(handle_type handler) : handler(handler) { }
    base_task(const awaiter_handler<T>& callback) : callback(callback)
    { }
    base_task(const base_task&) = delete;
    base_task(base_task&& r) noexcept : handler(r.handler)
    {
        r.handler = nullptr;
    }
    ~base_task()
    {
        if (this->handler)
        {
            if (!this->done())
                throw std::runtime_error("cannot destroy handler before job does not finish");
            this->handler.destroy();
        }
    }

    bool await_ready()
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> raw)
    {
        auto h = std::coroutine_handle<base_promise>::from_address(raw.address());

        auto& promise = this->handler.promise();
        if (promise.error)
            std::rethrow_exception(this->handler.promise().error);

        this->handler.promise().parent = h;
        h.promise().child = this->handler;

        if(this->callback)
            this->callback(static_cast<task<T>&>(*this));
        else
            this->resume();
    }

    void resume()
    {
        if (this->done())
        {
            auto& parent = this->handler.promise().parent;
            if (parent)
                parent.resume();
        }
        else
        {
            this->handler.resume();
        }
    }

    void error(std::exception_ptr&& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = e;
    }

    void error(const std::exception& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
    }

    void error(std::exception&& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
    }

    void resume(std::exception_ptr&& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = e;
        this->resume();
    }

    void resume(const std::exception& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
        this->resume();
    }

    void resume(std::exception&& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
        this->resume();
    }

    void error(const boost::system::error_code& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
    }

    void error(boost::system::error_code&& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
    }

    void resume(const boost::system::error_code& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
        this->resume();
    }

    void resume(boost::system::error_code&& e)
    {
        auto& promise = static_cast<base_promise&>(this->handler.promise());
        promise.error = std::make_exception_ptr(e);
        this->resume();
    }

    bool done()
    {
        if (!this->handler)
            throw std::runtime_error("destroyed task");

        return this->handler.done();
    }

    void operator = (const base_task&) = delete;
    void operator = (base_task&& r) noexcept
    {
        this->handler = r.handler;
        r.handler = nullptr;
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

template <typename T = void>
class task : public base_task<T>
{
public:
    class promise_type;

public:
    task(handle_type handler) : base_task<T>(handler)
    {}
    task(awaiter_handler<T> callback) : base_task<T>(callback)
    {}
    task(const task&) = delete;
    task(task&& r) : base_task<T>(static_cast<base_task<T>&&>(r))
    {}

public:
    using base_task<T>::resume;

public:
    T& value()
    {
        auto& promise = static_cast<promise_type&>(this->handler.promise());
        if (promise.error)
            std::rethrow_exception(promise.error);

        if (promise.value.has_value() == false)
            throw std::runtime_error("value is empty");

        if constexpr (std::is_class_v<T>)
        {
            return promise.value.value().get();
        }
        else
        {
            return promise.value.value();
        }
    }

    T& await_resume()
    {
        auto& promise = this->handler.promise();
        if (promise.parent)
            promise.parent.promise().child = nullptr;

        return this->value();
    }

    void resume(T&& value)
    {
        auto& promise = static_cast<promise_type&>(this->handler.promise());
        promise.value = value;
        base_task<T>::resume();
    }

    void resume(T& value)
    {
        auto& promise = static_cast<promise_type&>(this->handler.promise());
        promise.value = std::ref(value);
        base_task<T>::resume();
    }

    void result(T&& value)
    {
        auto& promise = static_cast<promise_type&>(this->handler.promise());
        promise.value = value;
    }

    void operator = (const task&) = delete;
    void operator = (task&& r)
    {
        base_task<T>::operator = (static_cast<base_task<T>&&>(r));
    }
};

template <>
class task<void> : public base_task<void>
{
public:
    class promise_type;

public:
    using base_task<void>::resume;

public:
    task(handle_type handler) : base_task<void>(handler)
    {}
    task(awaiter_handler<void> callback) : base_task<void>(callback)
    {}
    task(const task&) = delete;
    task(task&& r) noexcept : base_task<void>(static_cast<base_task<void>&&>(r))
    {}

public:
    void await_resume()
    {
        auto& promise = this->handler.promise();
        if (promise.parent)
            promise.parent.promise().child = nullptr;
    }

    void operator = (const task&) = delete;
    void operator = (task&& r)
    {
        base_task<void>::operator = (static_cast<base_task<void>&&>(r));
    }
};

template <typename T>
class task<T>::promise_type : public base_promise
{
public:
    task_result<T>::type value;

    auto get_return_object() { return task<T>(handle_type::from_promise(*this)); }

    template<std::convertible_to<T> From>
    void return_value(From&& from)
    {
        this->value = std::forward<From>(from);
    }
};

class task<void>::promise_type : public base_promise
{
public:
    auto get_return_object() { return task<void>(handle_type::from_promise(*this)); }
    void return_void() { }
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
        auto& base_promise = coro.base_promise();
        if (base_promise.has_value() == false)
            throw std::runtime_error("has no value");

        return base_promise.current_value.value();
    }

    void send(const INPUT& input)
    {
        coro.base_promise().input_value = input;
    }

    void send(INPUT&& input)
    {
        coro.base_promise().input_value = std::move(input);
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
        auto& base_promise = coro.base_promise();
        if (base_promise.current_value.has_value() == false)
            throw std::runtime_error("has no value");

        return base_promise.current_value.value();
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

template <typename T>
using awaiter = task<T>;

}

#endif