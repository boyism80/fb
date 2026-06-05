#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <shared_mutex>
#include <functional>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

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
public:
    class write_guard
    {
    public:
        write_guard(write_guard&&) noexcept         = default;
        write_guard& operator= (write_guard&&)      = delete;
        write_guard(const write_guard&)             = delete;
        write_guard& operator= (const write_guard&) = delete;

        ValueType& value() noexcept
        {
            return this->_owner->_value;
        }

    private:
        friend class locker;

        locker*                             _owner;
        std::unique_lock<std::shared_mutex> _lock;

        write_guard(locker& owner, std::unique_lock<std::shared_mutex>&& lock) noexcept :
            _owner(&owner),
            _lock(std::move(lock))
        { }
    };

    class read_guard
    {
    public:
        read_guard(read_guard&&) noexcept         = default;
        read_guard& operator= (read_guard&&)      = delete;
        read_guard(const read_guard&)             = delete;
        read_guard& operator= (const read_guard&) = delete;

        const ValueType& value() const noexcept
        {
            return this->_owner->_value;
        }

    private:
        friend class locker;

        const locker*                       _owner;
        std::shared_lock<std::shared_mutex> _lock;

        read_guard(const locker& owner, std::shared_lock<std::shared_mutex>&& lock) noexcept :
            _owner(&owner),
            _lock(std::move(lock))
        { }
    };

    class async_write_guard
    {
    public:
        async_write_guard(async_write_guard&& other) noexcept :
            _owner(other._owner),
            _lock(std::move(other._lock)),
            _async_locked(other._async_locked)
        {
            other._async_locked = false;
        }

        async_write_guard& operator= (async_write_guard&& other) noexcept
        {
            if (this != &other)
            {
                this->release();
                this->_owner        = other._owner;
                this->_lock         = std::move(other._lock);
                this->_async_locked = other._async_locked;
                other._async_locked = false;
            }
            return *this;
        }

        async_write_guard(const async_write_guard&)             = delete;
        async_write_guard& operator= (const async_write_guard&) = delete;

        ~async_write_guard()
        {
            this->release();
        }

        ValueType& value() noexcept
        {
            return this->_owner->_value;
        }

    private:
        friend class locker;

        locker*                             _owner = nullptr;
        std::unique_lock<std::shared_mutex> _lock;
        bool                                _async_locked = false;

        async_write_guard(locker& owner, std::unique_lock<std::shared_mutex>&& lock, bool async_locked) noexcept :
            _owner(&owner),
            _lock(std::move(lock)),
            _async_locked(async_locked)
        { }

        void release() noexcept
        {
            if (this->_async_locked == false)
                return;

            if (this->_lock.owns_lock())
                this->_lock.unlock();

            if (this->_owner != nullptr)
                this->_owner->_async_mutex.unlock();

            this->_async_locked = false;
        }
    };

    class async_read_guard
    {
    public:
        async_read_guard(async_read_guard&& other) noexcept :
            _owner(other._owner),
            _lock(std::move(other._lock)),
            _async_locked(other._async_locked)
        {
            other._async_locked = false;
        }

        async_read_guard& operator= (async_read_guard&& other) noexcept
        {
            if (this != &other)
            {
                this->release();
                this->_owner        = other._owner;
                this->_lock         = std::move(other._lock);
                this->_async_locked = other._async_locked;
                other._async_locked = false;
            }
            return *this;
        }

        async_read_guard(const async_read_guard&)             = delete;
        async_read_guard& operator= (const async_read_guard&) = delete;

        ~async_read_guard()
        {
            this->release();
        }

        const ValueType& value() const noexcept
        {
            return this->_owner->_value;
        }

    private:
        friend class locker;

        const locker*                       _owner = nullptr;
        std::shared_lock<std::shared_mutex> _lock;
        bool                                _async_locked = false;

        async_read_guard(const locker& owner, std::shared_lock<std::shared_mutex>&& lock, bool async_locked) noexcept :
            _owner(&owner),
            _lock(std::move(lock)),
            _async_locked(async_locked)
        { }

        void release() noexcept
        {
            if (this->_async_locked == false)
                return;

            if (this->_lock.owns_lock())
                this->_lock.unlock();

            if (this->_owner != nullptr)
                this->_owner->_async_mutex.unlock_shared();

            this->_async_locked = false;
        }
    };

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

    write_guard enter_write()
    {
        return write_guard(*this, std::unique_lock(this->_sync_mutex));
    }

    read_guard enter_read() const
    {
        return read_guard(*this, std::shared_lock(this->_sync_mutex));
    }

    async::task<async_write_guard> enter_write_async()
    {
        co_await this->_async_mutex.lock();

        try
        {
            co_return async_write_guard(*this, std::unique_lock(this->_sync_mutex), true);
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }

    async::task<async_read_guard> enter_read_async() const
    {
        co_await this->_async_mutex.lock_shared();

        try
        {
            co_return async_read_guard(*this, std::shared_lock(this->_sync_mutex), true);
        }
        catch (...)
        {
            this->_async_mutex.unlock_shared();
            throw;
        }
    }

    async::task<std::optional<async_write_guard>> try_enter_write_async()
    {
        if (this->_async_mutex.try_lock() == false)
            co_return std::nullopt;

        try
        {
            co_return async_write_guard(*this, std::unique_lock(this->_sync_mutex), true);
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }

    async::task<std::optional<async_read_guard>> try_enter_read_async() const
    {
        if (this->_async_mutex.try_lock_shared() == false)
            co_return std::nullopt;

        try
        {
            co_return async_read_guard(*this, std::shared_lock(this->_sync_mutex), true);
        }
        catch (...)
        {
            this->_async_mutex.unlock_shared();
            throw;
        }
    }

    template <typename Func> auto write(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
    {
        auto guard = this->enter_write();
        return fn(guard.value());
    }

    template <typename Func> auto read(Func&& fn) const -> decltype(fn(std::declval<const ValueType&>()))
    {
        auto guard = this->enter_read();
        return fn(guard.value());
    }

    template <typename Func> auto async_read(Func&& fn) const -> decltype(fn(std::declval<const ValueType&>()))
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));
        auto guard       = co_await this->enter_read_async();

        if constexpr (std::is_same_v<decltype((*func_holder)(guard.value())), async::task<void>>)
            co_await (*func_holder)(guard.value());
        else
            co_return co_await (*func_holder)(guard.value());
    }

    template <typename Func> auto async_write(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));
        auto guard       = co_await this->enter_write_async();

        if constexpr (std::is_same_v<decltype((*func_holder)(guard.value())), async::task<void>>)
            co_await (*func_holder)(guard.value());
        else
            co_return co_await (*func_holder)(guard.value());
    }

    template <typename Func> auto try_async_read(Func&& fn) const -> async::task<bool>
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));
        auto guard       = co_await this->try_enter_read_async();

        if (guard.has_value() == false)
            co_return false;

        if constexpr (std::is_same_v<decltype((*func_holder)(guard->value())), async::task<void>>)
            co_await (*func_holder)(guard->value());
        else
            std::ignore = co_await (*func_holder)(guard->value());

        co_return true;
    }

    template <typename Func> auto try_async_write(Func&& fn) -> async::task<bool>
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(fn));
        auto guard       = co_await this->try_enter_write_async();

        if (guard.has_value() == false)
            co_return false;

        if constexpr (std::is_same_v<decltype((*func_holder)(guard->value())), async::task<void>>)
            co_await (*func_holder)(guard->value());
        else
            std::ignore = co_await (*func_holder)(guard->value());

        co_return true;
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
