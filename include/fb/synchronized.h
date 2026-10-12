#ifndef __SYNCHRONIZED_H__
#define __SYNCHRONIZED_H__

// Microsoft cpp-async library
#include <async/task.h>
#include <async/task_completion_source.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

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
    int                                                              _count;
    std::mutex                                                       _mutex;
    std::queue<std::shared_ptr<async::task_completion_source<void>>> _waiters;

    // The count check and waiter registration happen under _mutex in one step;
    // otherwise release() could see a pending acquirer before it is queued and pop an empty queue.
    async::task<void> acquire()
    {
        std::shared_ptr<async::task_completion_source<void>> tcs;
        {
            std::lock_guard lk(this->_mutex);
            if (this->_count > 0)
            {
                this->_count--;
                co_return;
            }

            tcs = std::make_shared<async::task_completion_source<void>>();
            this->_waiters.push(tcs);
        }
        co_await tcs->task();
    }

    void release()
    {
        std::shared_ptr<async::task_completion_source<void>> next;
        {
            std::lock_guard lk(this->_mutex);
            if (this->_waiters.empty())
            {
                this->_count++;
            }
            else
            {
                // Hand the permit straight to the next waiter without returning it to the count.
                next = this->_waiters.front();
                this->_waiters.pop();
            }
        }

        if (next)
            next->set_value();
    }
};

class async_shared_mutex
{
public:
    using scheduler_type = std::function<void(std::function<void()>)>;

    // Each logic thread installs its scheduler here. A waiter resumes through the scheduler of the
    // thread it waited on, never inline on the releasing thread.
    static inline thread_local scheduler_type* thread_scheduler = nullptr;

private:
    struct waiter
    {
        std::shared_ptr<async::task_completion_source<void>> tcs;
        scheduler_type*                                      scheduler;
    };

public:
    // Every state check and waiter registration happens under _mutex in one step.
    // Checking outside it let an unlock slip in between and leave the new waiter without a wakeup.
    //
    // Release never hands the lock to a waiter; it only wakes waiters, which retry on their own thread.
    // A waiter therefore holds nothing while it waits for its thread, so a thread blocking on the
    // same lock synchronously cannot deadlock with it.
    async::task<void> lock_shared()
    {
        while (true)
        {
            auto tcs = std::make_shared<async::task_completion_source<void>>();
            {
                std::lock_guard lk(this->_mutex);
                if (this->_writer == false && this->_writers_waiting == 0)
                {
                    this->_reader_count++;
                    co_return;
                }

                this->_waiters.push_back(waiter{tcs, thread_scheduler});
            }
            co_await tcs->task();
        }
    }

    void unlock_shared()
    {
        auto waiters = std::vector<waiter>{};
        {
            std::lock_guard lk(this->_mutex);
            this->_reader_count--;
            if (this->_reader_count == 0)
                waiters = std::exchange(this->_waiters, {});
        }
        wake(waiters);
    }

    async::task<void> lock()
    {
        {
            std::lock_guard lk(this->_mutex);
            this->_writers_waiting++;
        }

        while (true)
        {
            auto tcs = std::make_shared<async::task_completion_source<void>>();
            {
                std::lock_guard lk(this->_mutex);
                if (this->_reader_count == 0 && this->_writer == false)
                {
                    this->_writer = true;
                    this->_writers_waiting--;
                    co_return;
                }

                this->_waiters.push_back(waiter{tcs, thread_scheduler});
            }
            co_await tcs->task();
        }
    }

    void unlock()
    {
        auto waiters = std::vector<waiter>{};
        {
            std::lock_guard lk(this->_mutex);
            this->_writer = false;
            waiters       = std::exchange(this->_waiters, {});
        }
        wake(waiters);
    }

    bool try_lock_shared()
    {
        std::lock_guard lk(this->_mutex);
        if (this->_writer == false && this->_writers_waiting == 0)
        {
            this->_reader_count++;
            return true;
        }
        return false;
    }

    bool try_lock()
    {
        std::lock_guard lk(this->_mutex);
        if (this->_reader_count == 0 && this->_writer == false)
        {
            this->_writer = true;
            return true;
        }
        return false;
    }

private:
    static void wake(std::vector<waiter>& waiters)
    {
        for (auto& entry : waiters)
        {
            if (entry.scheduler != nullptr)
            {
                (*entry.scheduler)([tcs = entry.tcs]() {
                    tcs->set_value();
                });
            }
            else
            {
                entry.tcs->set_value();
            }
        }
    }

private:
    int                 _reader_count    = 0;
    int                 _writers_waiting = 0;
    bool                _writer          = false;
    mutable std::mutex  _mutex;
    std::vector<waiter> _waiters;
};

template <typename ValueType>
class synchronized
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
        friend class synchronized;

        synchronized*                       _owner;
        std::unique_lock<std::shared_mutex> _lock;

        write_guard(synchronized& owner, std::unique_lock<std::shared_mutex>&& lock) noexcept :
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
        friend class synchronized;

        const synchronized*                 _owner;
        std::shared_lock<std::shared_mutex> _lock;

        read_guard(const synchronized& owner, std::shared_lock<std::shared_mutex>&& lock) noexcept :
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

        const ValueType& value() const noexcept
        {
            return this->_owner->_value;
        }

    private:
        friend class synchronized;

        synchronized*                       _owner = nullptr;
        std::unique_lock<std::shared_mutex> _lock;
        bool                                _async_locked = false;

        async_write_guard(synchronized& owner, std::unique_lock<std::shared_mutex>&& lock, bool async_locked) noexcept :
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
        friend class synchronized;

        const synchronized*                 _owner = nullptr;
        std::shared_lock<std::shared_mutex> _lock;
        bool                                _async_locked = false;

        async_read_guard(const synchronized&                   owner,
                         std::shared_lock<std::shared_mutex>&& lock,
                         bool                                  async_locked) noexcept :
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
    template <typename... Args> synchronized(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    synchronized(const synchronized&) = delete;
    synchronized(synchronized&&)      = delete;
    ~synchronized()                   = default;

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
class recursive_synchronized
{
private:
    mutable std::recursive_mutex _mutex;
    ValueType                    _value;

public:
    template <typename... Args> recursive_synchronized(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    recursive_synchronized(const recursive_synchronized&) = delete;
    recursive_synchronized(recursive_synchronized&&)      = delete;
    ~recursive_synchronized()                             = default;
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
