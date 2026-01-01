#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <shared_mutex>
#include <functional>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory>
#include <type_traits>

// Microsoft cpp-async library
#include <async/task.h>
#include <async/task_completion_source.h>

namespace fb {

class async_lock
{
public:
    explicit async_lock(int initial = 1) :
        _count(initial)
    { }

    template <typename T> async::task<T> enter(std::function<async::task<T>()> func)
    {
        co_await this->acquire();
        try
        {
            T result = co_await func();
            this->release();
            co_return result;
        }
        catch (...)
        {
            this->release();
            throw;
        }
    }

private:
    std::atomic<int>                                                 _count;
    std::mutex                                                       _mutex;
    std::queue<std::shared_ptr<async::task_completion_source<void>>> _waiters;

    async::task<void> acquire()
    {
        auto prev = this->_count.fetch_sub(1, std::memory_order_acquire);
        if (prev > 0)
            co_return;

        auto tcs = std::make_shared<async::task_completion_source<void>>();
        {
            std::lock_guard lk(this->_mutex);
            this->_waiters.push(tcs);
        }
        co_await tcs->task();
    }

    void release()
    {
        auto prev = this->_count.fetch_add(1, std::memory_order_release);
        if (prev < 0)
        {
            std::shared_ptr<async::task_completion_source<void>> next;
            {
                std::lock_guard lk(this->_mutex);
                next = this->_waiters.front();
                this->_waiters.pop();
            }
            next->set_value();
        }
    }
};

class async_shared_mutex
{
public:
    async::task<void> lock_shared()
    {
        if (!this->_writer.load(std::memory_order_acquire) && this->writer_queue_empty())
        {
            this->_reader_count.fetch_add(1, std::memory_order_relaxed);
            co_return;
        }

        auto tcs = std::make_shared<async::task_completion_source<void>>();
        {
            std::lock_guard lk(this->_mutex);
            this->_reader_waiters.push(tcs);
        }
        co_await tcs->task();
    }

    void unlock_shared()
    {
        if (this->_reader_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
            this->notify_writer();
    }

    async::task<void> lock()
    {
        auto expected = false;
        if (this->_reader_count.load(std::memory_order_acquire) == 0 &&
            this->_writer.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
        {
            co_return;
        }

        auto tcs = std::make_shared<async::task_completion_source<void>>();
        {
            std::lock_guard lk(this->_mutex);
            this->_writer_waiters.push(tcs);
        }
        co_await tcs->task();
    }

    void unlock()
    {
        this->_writer.store(false, std::memory_order_release);
        std::lock_guard lk(this->_mutex);

        if (!this->_writer_waiters.empty())
        {
            auto next = this->_writer_waiters.front();
            this->_writer_waiters.pop();
            this->_writer.store(true, std::memory_order_release);
            next->set_value();
            return;
        }

        while (!this->_reader_waiters.empty())
        {
            auto reader = this->_reader_waiters.front();
            this->_reader_waiters.pop();
            this->_reader_count.fetch_add(1, std::memory_order_relaxed);
            reader->set_value();
        }
    }

    bool try_lock_shared()
    {
        if (!this->_writer.load(std::memory_order_acquire) && this->writer_queue_empty())
        {
            this->_reader_count.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    bool try_lock()
    {
        auto expected = false;
        if (this->_reader_count.load(std::memory_order_acquire) == 0 &&
            this->_writer.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
        {
            return true;
        }
        return false;
    }

private:
    std::atomic<int>                                                 _reader_count{0};
    std::atomic<bool>                                                _writer{false};
    mutable std::mutex                                               _mutex;
    std::queue<std::shared_ptr<async::task_completion_source<void>>> _reader_waiters;
    std::queue<std::shared_ptr<async::task_completion_source<void>>> _writer_waiters;

    bool writer_queue_empty() const
    {
        std::lock_guard lk(this->_mutex);
        return this->_writer_waiters.empty();
    }

    void notify_writer()
    {
        std::shared_ptr<async::task_completion_source<void>> writer;
        {
            std::lock_guard lk(this->_mutex);
            if (!this->_writer_waiters.empty())
            {
                writer = this->_writer_waiters.front();
                this->_writer_waiters.pop();
                this->_writer.store(true, std::memory_order_release);
            }
        }

        if (writer)
            writer->set_value();
    }
};

template <typename ValueType>
class locker
{
private:
    mutable std::shared_mutex  _sync_mutex;
    mutable async_shared_mutex _async_mutex;
    ValueType                  _value;

public:
    template <typename... Args> locker(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    locker(const locker&) = delete;
    locker(locker&&)      = delete;
    ~locker()             = default;
    template <typename Func> auto write(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
    {
        auto _ = std::unique_lock(this->_sync_mutex);
        return fn(this->_value);
    }

    template <typename Func> auto read(Func&& fn) const -> decltype(fn(std::declval<const ValueType&>()))
    {
        auto _ = std::shared_lock(this->_sync_mutex);
        return fn(this->_value);
    }

    template <typename Func> auto async_read(Func&& fn) const -> decltype(fn(std::declval<const ValueType&>()))
    {
        // Create shared_ptr holder to ensure function lifetime safety
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));

        co_await this->_async_mutex.lock_shared();
        std::shared_lock lock(this->_sync_mutex);

        try
        {
            if constexpr (std::is_same_v<decltype((*func_holder)(this->_value)), async::task<void>>)
            {
                co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock_shared();
            }
            else
            {
                auto result = co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock_shared();
                co_return result;
            }
        }
        catch (...)
        {
            this->_async_mutex.unlock_shared();
            throw;
        }
    }

    template <typename Func> auto async_write(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
    {
        // Create shared_ptr holder to ensure function lifetime safety
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));

        co_await this->_async_mutex.lock();
        std::unique_lock lock(this->_sync_mutex);

        try
        {
            if constexpr (std::is_same_v<decltype((*func_holder)(this->_value)), async::task<void>>)
            {
                co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock();
            }
            else
            {
                auto result = co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock();
                co_return result;
            }
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }

    template <typename Func> auto try_async_read(Func&& fn) const -> async::task<bool>
    {
        // Create shared_ptr holder to ensure function lifetime safety
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));

        if (!this->_async_mutex.try_lock_shared())
        {
            co_return false;
        }

        std::shared_lock lock(this->_sync_mutex);

        try
        {
            if constexpr (std::is_same_v<decltype((*func_holder)(this->_value)), async::task<void>>)
            {
                co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock_shared();
                co_return true;
            }
            else
            {
                co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock_shared();
                co_return true;
            }
        }
        catch (...)
        {
            this->_async_mutex.unlock_shared();
            throw;
        }
    }

    template <typename Func> auto try_async_write(Func&& fn) -> async::task<bool>
    {
        // Create shared_ptr holder to ensure function lifetime safety
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));

        if (!this->_async_mutex.try_lock())
        {
            co_return false;
        }

        std::unique_lock lock(this->_sync_mutex);

        try
        {
            if constexpr (std::is_same_v<decltype((*func_holder)(this->_value)), async::task<void>>)
            {
                co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock();
                co_return true;
            }
            else
            {
                co_await (*func_holder)(this->_value);
                this->_async_mutex.unlock();
                co_return true;
            }
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }
};

template <typename ValueType>
class recursive_locker
{
private:
    mutable std::recursive_mutex _mutex;
    ValueType                    _value;

public:
    template <typename... Args> recursive_locker(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    recursive_locker(const recursive_locker&) = delete;
    recursive_locker(recursive_locker&&)      = delete;
    ~recursive_locker()                       = default;
    template <typename Func> auto lock(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
    {
        std::lock_guard<std::recursive_mutex> lock(this->_mutex);

        if constexpr (std::is_void_v<decltype(fn(this->_value))>)
        {
            fn(this->_value);
        }
        else
        {
            return fn(this->_value);
        }
    }

    template <typename Func>
    auto write(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
    {
        std::lock_guard<std::recursive_mutex> lock(this->_mutex);

        if constexpr (std::is_void_v<decltype(fn(this->_value))>)
        {
            fn(this->_value);
        }
        else
        {
            return fn(this->_value);
        }
    }

    template <typename Func>
    auto read(Func&& fn) const -> decltype(fn(std::declval<const ValueType&>()))
    {
        std::lock_guard<std::recursive_mutex> lock(this->_mutex);

        if constexpr (std::is_void_v<decltype(fn(this->_value))>)
        {
            fn(this->_value);
        }
        else
        {
            return fn(this->_value);
        }
    }
};

} // namespace fb

#endif
