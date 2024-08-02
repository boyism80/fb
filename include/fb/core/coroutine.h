#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <exception>
#include <stdexcept>
#include <optional>
#include <coroutine>
#include <functional>
#include <chrono>
#ifdef KINGDOM_OF_THE_WIND
#include <boost/system/error_code.hpp>
#endif

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
struct task_resume_type<T, typename std::enable_if_t<std::is_class_v<T>>>
{
    typedef T&& type;
};

template <typename T>
struct task_resume_type<T, typename std::enable_if_t<!std::is_class_v<T>>>
{
    typedef T type;
};


class base_awaiter
{
private:
    std::exception_ptr      _error = nullptr;
    std::coroutine_handle<> _parent;

protected:
    base_awaiter()
    {
        printf("create base awaiter 0x%X\n", this);
    };
    base_awaiter(const base_awaiter&) = delete;
    base_awaiter(base_awaiter&& r) noexcept : _error(r._error), _parent(r._parent)
    {
        printf("create base awaiter(move) 0x%X\n", this);
    }

public:
    virtual ~base_awaiter()
    {
        printf("destroy base awaiter 0x%X\n", this);
    }

public:
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h)
    {
        _parent = h;
    }

    void await_resume()
    {
        if (this->_error)
            std::rethrow_exception(this->_error);
    }

    void resume()
    {
        if (this->_parent)
            this->_parent.resume();
    }

    void error(const std::exception& e)
    {
        this->_error = std::make_exception_ptr(e);
    }

    void error(std::exception&& e)
    {
        this->_error = std::make_exception_ptr(e);
    }

    void resume(std::exception_ptr&& e)
    {
        this->_error = e;
        this->resume();
    }

    void resume(const std::exception& e)
    {
        this->_error = std::make_exception_ptr(e);
        this->resume();
    }

    void resume(std::exception&& e)
    {
        this->_error = std::make_exception_ptr(e);
        this->resume();
    }

#ifdef KINGDOM_OF_THE_WIND
    void error(const boost::system::error_code& e)
    {
        this->_error = std::make_exception_ptr(e);
    }

    void error(boost::system::error_code&& e)
    {
        this->_error = std::make_exception_ptr(e);
    }

    void resume(const boost::system::error_code& e)
    {
        this->_error = std::make_exception_ptr(e);
        this->resume();
    }

    void resume(boost::system::error_code&& e)
    {
        this->_error = std::make_exception_ptr(e);
        this->resume();
    }
#endif

    void operator = (const base_awaiter&) = delete;
    void operator = (base_awaiter&& r) noexcept
    {
        this->_error = r._error;
        this->_parent = r._parent;
    }
};

template <typename T = void>
class awaiter : public base_awaiter
{
public:
    using handler = std::function<void(awaiter&)>;

private:
    task_result<T>::type    _value;
    handler                 _handler;

public:
    awaiter(const handler& handler) : _handler(handler)
    {}
    awaiter(const awaiter&) = delete;
    awaiter(awaiter&& r) : base_awaiter(static_cast<base_awaiter&&>(r)), _value(r._value), _handler(r._handler) {}
    ~awaiter() {}

public:
    using base_awaiter::resume;
    using base_awaiter::error;

public:
    void await_suspend(std::coroutine_handle<> h)
    {
        base_awaiter::await_suspend(h);
        _handler(*this);
    }

    T& await_resume()
    {
        base_awaiter::await_resume();

        if (!this->_value.has_value())
            throw std::runtime_error("value is empty");

        return this->_value.value();
    }

    template <typename Q = T>
    typename std::enable_if<std::is_class<Q>::value>::type resume(T&& value)
    {
        this->result(value);
        base_awaiter::resume();
    }

    template <typename Q = T>
    typename std::enable_if<std::is_class<Q>::value>::type resume(T& value)
    {
        this->result(value);
        base_awaiter::resume();
    }

    template <typename Q = T>
    typename std::enable_if<std::is_class<Q>::value>::type result(T&& value)
    {
        this->_value = value;
    }

    template <typename Q = T>
    typename std::enable_if<std::is_class<Q>::value>::type result(T& value)
    {
        this->_value = std::ref(value);
    }

    template <typename Q = T>
    typename std::enable_if<!std::is_class<Q>::value>::type resume(T value)
    {
        this->result(value);
        base_awaiter::resume();
    }

    template <typename Q = T>
    typename std::enable_if<!std::is_class<Q>::value>::type result(T value)
    {
        this->_value = value;
    }

    void operator = (const awaiter&) = delete;
    void operator = (awaiter&& r) noexcept
    {
        base_awaiter::operator = (static_cast<base_awaiter&&>(r));
        this->_handler = r._handler;
    }
};

template <>
class awaiter<void> : public base_awaiter
{
public:
    using handler = std::function<void(awaiter&)>;

private:
    handler _handler;

public:
    awaiter(const handler& handler) : _handler(handler)
    { }
    awaiter(const awaiter&) = delete;
    awaiter(awaiter&& r) noexcept : base_awaiter(static_cast<base_awaiter&&>(r)), _handler(r._handler) {}
    ~awaiter() = default;

public:
    void await_suspend(std::coroutine_handle<> h)
    {
        base_awaiter::await_suspend(h);
        _handler(*this);
    }
    void await_resume()
    {
        base_awaiter::await_resume();
    }

    void operator = (const awaiter&) = delete;
    void operator = (awaiter&& r) noexcept
    {
        base_awaiter::operator = (static_cast<base_awaiter&&>(r));
        this->_handler = r._handler;
    }
};


class base_promise;
using handle_type = std::coroutine_handle<base_promise>;

class base_promise
{
public:
    std::exception_ptr          error = nullptr;
    std::coroutine_handle<>     parent;
    bool                        finished = false;

    auto initial_suspend() { return std::suspend_never{}; }
    auto final_suspend() noexcept
    {
        printf("final suspend from task (promise : 0x%X)\n", this);
        return std::suspend_always{};
    }
    void unhandled_exception() { this->error = std::current_exception(); }
};

template <typename T> class task;

template <typename T>
class base_task
{
public:
    handle_type handler;

public:
    base_task(handle_type handler) : handler(handler)
    {
        printf("create base task 0x%X\n", this);
    }
    base_task(const base_task&) = delete;
    base_task(base_task&& r) noexcept : handler(r.handler)
    {
        printf("create base task(move) 0x%X\n", this);
        r.handler = nullptr;
    }
    ~base_task()
    {
        printf("destroy base task 0x%X\n", this);
        if (this->handler)
        {
            if (!this->done())
            {
                auto sstream = std::stringstream();
                auto address = this->handler.address();
                sstream
                    << "cannot destroy handler before job does not finish : "
                    << address;
                throw std::runtime_error(sstream.str());
            }
            this->handler.destroy();
        }
    }

    bool await_ready()
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> raw)
    {
        auto& promise = this->handler.promise();
        if (promise.error)
            std::rethrow_exception(promise.error);

        //promise.parent = raw;
        //this->resume();
        if (this->done())
            raw.resume();
        else
            promise.parent = raw;
    }

    bool done()
    {
        if (!this->handler)
            throw std::runtime_error("destroyed task");

        if (this->handler.done())
            return true;

        auto& promise = this->handler.promise();
        return promise.finished;
    }

    void operator = (const base_task&) = delete;
    void operator = (base_task&& r) noexcept
    {
        this->handler = r.handler;
        r.handler = nullptr;
    }
};

template <typename T = void>
class task : public base_task<T>
{
public:
    class promise_type;

private:
    std::optional<fb::awaiter<T>> _awaiter;

public:
    task(handle_type handler) : base_task<T>(handler) {}
    task(const task&) = delete;
    task(task&& r) : base_task<T>(static_cast<base_task<T>&&>(r)) {}

//public:
//    using base_task<T>::resume;

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
        if(this->_awaiter.has_value())
        {
            this->_awaiter.value().resume(this->value());
            this->_awaiter.reset();
        }
        return this->value();
    }

    T& wait()
    {
        constexpr auto term = 100ms;
        while(!this->done())
        {
            std::this_thread::sleep_for(term);
        }

        return this->value();
    }

    void set_awaiter(fb::awaiter<T>&& awaiter)
    {
        this->_awaiter = std::move(awaiter);
        if(this->done())
        {
            this->_awaiter.value().resume(this->value());
            this->_awaiter.reset();
        }
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

private:
    std::optional<fb::awaiter<void>> _awaiter;

//public:
//    using base_task<void>::resume;

public:
    task(handle_type handler) : base_task<void>(handler) {}
    task(const task&) = delete;
    task(task&& r) noexcept : base_task<void>(static_cast<base_task<void>&&>(r)) {}

public:
    void await_resume()
    {
        if(this->_awaiter.has_value())
        {
            this->_awaiter.value().resume();
            this->_awaiter.reset();
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

    void set_awaiter(fb::awaiter<void>&& awaiter)
    {
        this->_awaiter = std::move(awaiter);
        if(this->done())
        {
            this->_awaiter.value().resume();
            this->_awaiter.reset();
        }
    }

    void operator = (const task&) = delete;
    void operator = (task&& r) noexcept
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
        this->finished = true;
        if (this->parent)
            this->parent.resume();
    }
};

class task<void>::promise_type : public base_promise
{
public:
    auto get_return_object() { return task<void>(handle_type::from_promise(*this)); }
    void return_void()
    {
        this->finished = true;
        if (this->parent)
            this->parent.resume();
    }
};

template <typename T>
class base_awaiter
{
protected:
    std::exception_ptr                      error;

public:
    fb::task<T, std::suspend_always>        task;

protected:
    base_awaiter(fb::task<T, std::suspend_always>&& task) : task(std::move(task))
    {}
    base_awaiter(const base_awaiter&) = delete;
    base_awaiter(base_awaiter&& r) : error(r.error), task(std::move(r.task))
    {
        r.error = nullptr;
    }
    ~base_awaiter() = default;

public:
    void set_error(const std::exception& e)
    {
        this->error = std::make_exception_ptr(e);
    }
};

template <typename T>
class awaiter : public base_awaiter<T>
{
private:
    fb::task_result<T>::type                result;

public:
    awaiter() : base_awaiter<T>(handler())
    {}
    awaiter(const awaiter&) = delete;
    awaiter(awaiter&& r) : base_awaiter<T>(static_cast<base_awaiter<T>&&>(r)), result(std::move(r.result))
    {}
    ~awaiter() = default;

private:
    fb::task<T, std::suspend_always> handler()
    {
        if (this->error != nullptr)
            std::rethrow_exception(this->error);

        if (this->result.has_value() == false)
            throw std::runtime_error("has no value");

        co_return result.value();
    }

public:
    template <typename Q = T>
    typename std::enable_if<std::is_class<Q>::value>::type set_result(T&& value)
    {
        this->result = value;
        this->task.handler.resume();
    }

    template <typename Q = T>
    typename std::enable_if<std::is_class<Q>::value>::type set_result(T& value)
    {
        this->result = value;
        this->task.handler.resume();
    }

    template <typename Q = T>
    typename std::enable_if<!std::is_class<Q>::value>::type set_result(T value)
    {
        this->result = value;
        this->task.handler.resume();
    }
};


template <>
class awaiter<void> : public base_awaiter<void>
{
public:
    awaiter() : base_awaiter<void>(handler())
    {}
    awaiter(const awaiter&) = delete;
    awaiter(awaiter&& r) : base_awaiter<void>(static_cast<base_awaiter<void>&&>(r))
    {}
    ~awaiter() = default;

private:
    fb::task<void, std::suspend_always> handler()
    {
        if (this->error != nullptr)
            std::rethrow_exception(this->error);

        co_return;
    }

public:
    void resume()
    {
        this->task.handler.resume();
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