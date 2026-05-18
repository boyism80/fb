#ifndef __ASYNC_GENERATOR_H__
#define __ASYNC_GENERATOR_H__

#include <async/task.h>
#include <async/task_completion_source.h>
#include <coroutine>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

namespace fb {

/// Suspend without yielding a value (for async_generator<void> only).
/// Use between steps so co_await gen.next() can resume on the next timer tick.
struct async_suspend
{ };

/// Async iterable coroutine: supports co_yield and co_await async::task inside the body.
/// Consumer advances with co_await gen.next() (not synchronous next()).
template <typename T>
class async_generator
{
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        std::optional<T>                                     current_value;
        std::exception_ptr                                   exception;
        bool                                                 yielded = false;
        std::shared_ptr<async::task_completion_source<void>> signal;

        auto get_return_object()
        {
            return async_generator{coro_handle::from_promise(*this)};
        }

        constexpr std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        constexpr std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        void unhandled_exception()
        {
            this->exception = std::current_exception();
            this->notify();
        }

        auto yield_value(T value)
        {
            this->current_value = std::move(value);
            this->yielded       = true;
            this->notify();
            return std::suspend_always{};
        }

        void return_void()
        {
            this->notify();
        }

        template <typename U>
        auto await_transform(async::task<U>&& task)
        {
            return std::forward<async::task<U>>(task);
        }

        template <typename U>
        auto await_transform(async::task<U>& task)
        {
            return task;
        }

        void prepare_advance()
        {
            this->yielded = false;
            this->current_value.reset();
            this->signal = std::make_shared<async::task_completion_source<void>>();
        }

        void notify()
        {
            if (this->signal != nullptr)
                this->signal->set_value();
        }
    };

public:
    [[nodiscard]] async::task<bool> next()
    {
        if (this->_coro == nullptr)
            co_return false;

        if (this->_coro.done())
        {
            this->rethrow_if_exception();
            co_return false;
        }

        auto& promise = this->_coro.promise();

        while (true)
        {
            promise.prepare_advance();
            this->_coro.resume();

            if (promise.yielded)
                co_return true;

            if (this->_coro.done())
            {
                this->rethrow_if_exception();
                co_return false;
            }

            co_await promise.signal->task();

            if (promise.yielded)
                co_return true;

            if (this->_coro.done())
            {
                this->rethrow_if_exception();
                co_return false;
            }
        }
    }

    [[nodiscard]] const T& value() const
    {
        auto& promise = this->_coro.promise();
        if (promise.current_value.has_value() == false)
            throw std::runtime_error("async_generator has no value");

        return promise.current_value.value();
    }

    [[nodiscard]] bool done() const noexcept
    {
        return this->_coro == nullptr || this->_coro.done();
    }

    async_generator(async_generator const&)             = delete;
    async_generator& operator= (async_generator const&) = delete;

    async_generator(async_generator&& rhs) noexcept :
        _coro(rhs._coro)
    {
        rhs._coro = nullptr;
    }

    async_generator& operator= (async_generator&& rhs) noexcept
    {
        if (this != &rhs)
        {
            if (this->_coro)
                this->_coro.destroy();

            this->_coro = rhs._coro;
            rhs._coro   = nullptr;
        }
        return *this;
    }

    ~async_generator()
    {
        if (this->_coro)
            this->_coro.destroy();
    }

private:
    explicit async_generator(coro_handle coro) :
        _coro(coro)
    { }

    void rethrow_if_exception() const
    {
        auto& promise = this->_coro.promise();
        if (promise.exception != nullptr)
            std::rethrow_exception(promise.exception);
    }

    coro_handle _coro = nullptr;
};

/// Async coroutine with no yielded values: co_await async::task and co_await async_suspend only.
template <>
class async_generator<void>
{
public:
    struct promise_type;

private:
    using coro_handle = std::coroutine_handle<promise_type>;

    struct suspend_awaiter
    {
        promise_type* _promise;

        constexpr bool await_ready() const noexcept
        {
            return false;
        }

        bool await_suspend(std::coroutine_handle<>)
        {
            this->_promise->step_completed = true;
            this->_promise->notify();
            return true;
        }

        constexpr void await_resume() const noexcept
        { }
    };

public:
    struct promise_type
    {
        std::exception_ptr                                   exception;
        bool                                                 step_completed = false;
        std::shared_ptr<async::task_completion_source<void>> signal;

        auto get_return_object()
        {
            return async_generator{coro_handle::from_promise(*this)};
        }

        constexpr std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        constexpr std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        void unhandled_exception()
        {
            this->exception = std::current_exception();
            this->notify();
        }

        void return_void()
        {
            this->notify();
        }

        suspend_awaiter await_transform(async_suspend)
        {
            return suspend_awaiter{this};
        }

        template <typename U>
        auto await_transform(async::task<U>&& task)
        {
            return std::forward<async::task<U>>(task);
        }

        template <typename U>
        auto await_transform(async::task<U>& task)
        {
            return task;
        }

        void prepare_advance()
        {
            this->step_completed = false;
            this->signal         = std::make_shared<async::task_completion_source<void>>();
        }

        void notify()
        {
            if (this->signal != nullptr)
                this->signal->set_value();
        }
    };

public:
    [[nodiscard]] async::task<bool> next()
    {
        if (this->_coro == nullptr)
            co_return false;

        if (this->_coro.done())
        {
            this->rethrow_if_exception();
            co_return false;
        }

        auto& promise = this->_coro.promise();

        while (true)
        {
            promise.prepare_advance();
            this->_coro.resume();

            if (promise.step_completed)
                co_return true;

            if (this->_coro.done())
            {
                this->rethrow_if_exception();
                co_return false;
            }

            co_await promise.signal->task();

            if (promise.step_completed)
                co_return true;

            if (this->_coro.done())
            {
                this->rethrow_if_exception();
                co_return false;
            }
        }
    }

    [[nodiscard]] bool done() const noexcept
    {
        return this->_coro == nullptr || this->_coro.done();
    }

    async_generator(async_generator const&)             = delete;
    async_generator& operator= (async_generator const&) = delete;

    async_generator(async_generator&& rhs) noexcept :
        _coro(rhs._coro)
    {
        rhs._coro = nullptr;
    }

    async_generator& operator= (async_generator&& rhs) noexcept
    {
        if (this != &rhs)
        {
            if (this->_coro)
                this->_coro.destroy();

            this->_coro = rhs._coro;
            rhs._coro   = nullptr;
        }
        return *this;
    }

    ~async_generator()
    {
        if (this->_coro)
            this->_coro.destroy();
    }

private:
    explicit async_generator(coro_handle coro) :
        _coro(coro)
    { }

    void rethrow_if_exception() const
    {
        auto& promise = this->_coro.promise();
        if (promise.exception != nullptr)
            std::rethrow_exception(promise.exception);
    }

    coro_handle _coro = nullptr;
};

} // namespace fb

namespace std {

/// Member-function coroutines returning fb::async_generator<void> (e.g. timer::delivery_coroutine).
template <typename... Args>
struct coroutine_traits<fb::async_generator<void>, Args...>
{
    using promise_type = fb::async_generator<void>::promise_type;
};

} // namespace std

#endif // !__ASYNC_GENERATOR_H__
