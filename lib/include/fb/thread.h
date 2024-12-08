#ifndef __THREAD_H__
#define __THREAD_H__

#include <boost/asio.hpp>
#include <thread>
#include <functional>
#include <map>
#include <future>
#include <atomic>
#include <fb/logger.h>
#include <fb/timer.h>
#include <fb/pqueue.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_then.h>
#include <fb/model/datetime.h>

#define MUTEX_GUARD(x) auto _ = std::lock_guard(x);

namespace fb {

using queue_callback = std::function<void(uint8_t)>;

class thread;
class threads;

class thread_switchable
{
protected:
    thread_switchable() = default;

public:
    virtual ~thread_switchable() = default;

public:
    virtual fb::thread* thread() const = 0;
};

class thread
{
public:
    using async_func_type = std::function<async::task<void>()>;
    using func_type       = std::function<void()>;

private:
    uint8_t           _index = 0;
    std::atomic<bool> _exit  = false;
    std::thread       _thread;

private:
    std::vector<std::unique_ptr<timer>> _timers;
    std::recursive_mutex                _mutex_timer;
    void*                               _data = nullptr;

private:
    std::queue<std::function<void()>> _queue;
    std::mutex                        _mutex_queue;

public:
    thread(uint8_t index);
    ~thread();

    thread(const thread&) = delete;
    thread(thread&&)      = delete;

    thread& operator= (thread&)       = delete;
    thread& operator= (const thread&) = delete;

private:
    void handle_thread(uint8_t index);
    void handle_idle();

public:
    std::thread::id id() const;
    uint8_t         index() const;
    void            exit();
    void            data(void* value);
    void*           data() const;
    template <typename ReturnType>
    ReturnType* data() const
    {
        return static_cast<ReturnType*>(this->_data);
    }

public:
    void settimer(const fb::timer_callback& fn, const fb::model::timespan& duration, bool disposable = false);
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration);

    template <typename ReturnType>
    void enqueue(const std::function<async::task<ReturnType>()>& fn,
                 const std::function<void(std::exception&)>&     error,
                 const std::function<void(ReturnType&& value)>&  callback)
    {
        this->_queue.push([=]() {
            async::awaitable_then(fn(), [&](async::awaitable_result<ReturnType> result) {
                try
                {
                    callback(result());
                }
                catch (std::exception& e)
                {
                    error(e);
                }
                catch (...)
                {
                    try
                    {
                        std::rethrow_exception(std::current_exception());
                    }
                    catch (std::exception& e)
                    {
                        error(e);
                    }
                }
            });
        });
    }

    void enqueue(const std::function<async::task<void>()>&   fn,
                 const std::function<void(std::exception&)>& error,
                 const std::function<void()>&                callback)
    {
        this->_queue.push([=]() {
            async::awaitable_then(fn(), [=](async::awaitable_result<void> result) {
                try
                {
                    callback();
                }
                catch (std::exception& e)
                {
                    error(e);
                }
                catch (...)
                {
                    try
                    {
                        std::rethrow_exception(std::current_exception());
                    }
                    catch (std::exception& e)
                    {
                        error(e);
                    }
                }
            });
        });
    }

    template <typename ReturnType>
    [[nodiscard]] async::task<ReturnType> dispatch(const std::function<async::task<ReturnType>()>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue<ReturnType>(
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        return promise->task();
    }

    [[nodiscard]] async::task<void> dispatch(const std::function<async::task<void>()>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue(
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise]() {
                promise->set_value();
            });
        return promise->task();
    }

    [[nodiscard]] async::task<void> switching()
    {
        return this->dispatch([]() -> async::task<void> {
            co_return;
        });
    }
};

class threads
{
public:
    using unique_thread  = std::unique_ptr<fb::thread>;
    using unique_threads = std::unordered_map<std::thread::id, unique_thread>;
    using unique_id_list = std::unique_ptr<std::thread::id[]>;

private:
    boost::asio::io_context& _context;
    unique_threads           _threads;
    unique_id_list           _keys;

public:
    threads(boost::asio::io_context& context);
    ~threads() = default;

    threads(const threads&) = delete;
    threads(threads&&)      = delete;

    threads& operator= (threads&)       = delete;
    threads& operator= (const threads&) = delete;

public:
    fb::thread*       at(uint8_t index) const;
    fb::thread*       at(std::thread::id id) const;
    fb::thread*       modular(uint32_t id) const;
    fb::thread*       current();
    const fb::thread* current() const;
    uint8_t           count() const;
    bool              empty() const;
    bool              valid(uint8_t index) const;
    bool              valid(fb::thread* thread) const;
    bool              valid(fb::thread& thread) const;
    size_t            size() const;

public:
    template <typename ReturnType>
    void enqueue(thread_switchable&                              pivot,
                 const std::function<bool()>&                    condition,
                 const std::function<async::task<ReturnType>()>& fn,
                 const std::function<void(std::exception&)>&     error,
                 const std::function<void(ReturnType&&)>&        callback)
    {
        auto thread = pivot.thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->enqueue<ReturnType>(
            [=, this]() -> async::task<void> {
                if (condition() == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread  = pivot.thread();
                auto current_thread = this->current();
                if (active_thread != current_thread)
                {
                    this->enqueue(pivot, condition, fn);
                    throw std::runtime_error("active thread not matched");
                }

                co_return co_await fn();
            },
            error,
            callback);
    }

    void enqueue(thread_switchable&                          pivot,
                 const std::function<bool()>&                condition,
                 const std::function<async::task<void>()>&   fn,
                 const std::function<void(std::exception&)>& error,
                 const std::function<void()>&                callback)
    {
        auto thread = pivot.thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->enqueue(
            [=, &pivot, this]() -> async::task<void> {
                if (condition() == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread  = pivot.thread();
                auto current_thread = this->current();
                if (active_thread != current_thread)
                {
                    this->enqueue(pivot, condition, fn);
                    throw std::runtime_error("active thread not matched");
                }

                co_return co_await fn();
            },
            error,
            callback);
    }

    template <typename ReturnType>
    void enqueue(thread_switchable&                              pivot,
                 const std::function<bool()>&                    condition,
                 const std::function<async::task<ReturnType>()>& fn)
    {
        return this->enqueue<ReturnType>(
            pivot,
            condition,
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    void enqueue(thread_switchable&                        pivot,
                 const std::function<bool()>&              condition,
                 const std::function<async::task<void>()>& fn)
    {
        return this->enqueue(
            pivot,
            condition,
            fn,
            [](std::exception& e) {
            },
            []() {
            });
    }

    template <typename ReturnType>
    void enqueue(thread_switchable& pivot, const std::function<async::task<ReturnType>()>& fn)
    {
        return this->enqueue(
            pivot,
            []() -> bool {
                return true;
            },
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    void enqueue(thread_switchable& pivot, const std::function<async::task<void>()>& fn)
    {
        return this->enqueue(
            pivot,
            []() -> bool {
                return true;
            },
            fn,
            [](std::exception& e) {
            },
            []() {
            });
    }

    template <typename ReturnType>
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                              pivot,
                                             const std::function<bool()>&                    condition,
                                             const std::function<async::task<ReturnType>()>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue(
            pivot,
            condition,
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        return promise->task();
    }

    [[nodiscard]] async::task<void> dispatch(thread_switchable&                        pivot,
                                             const std::function<bool()>&              condition,
                                             const std::function<async::task<void>()>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue(
            pivot,
            condition,
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise]() {
                promise->set_value();
            });
        return promise->task();
    }

    template <typename ReturnType>
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                              pivot,
                                             const std::function<async::task<ReturnType>()>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue(
            pivot,
            []() -> bool {
                return true;
            },
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        return promise->task();
    }

    [[nodiscard]] async::task<void> dispatch(thread_switchable& pivot, const std::function<async::task<void>()>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue(
            pivot,
            []() -> bool {
                return true;
            },
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise]() {
                promise->set_value();
            });
        return promise->task();
    }

    [[nodiscard]] async::task<void> switching(thread_switchable& pivot)
    {
        co_await this->dispatch(pivot, []() -> async::task<void> {
            co_return;
        });
    }

    void settimer(const fb::timer_callback& fn, const fb::model::timespan& duration);
    void exit();

public:
    fb::thread* operator[] (uint8_t index) const;
    fb::thread* operator[] (std::thread::id id) const;
};

} // namespace fb

#endif // !__THREAD_H__
