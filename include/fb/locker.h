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

/**
 * @brief   Asynchronous exclusive lock for coroutine synchronization
 *
 *          Provides exclusive access control for asynchronous operations
 *          using semaphore-like counting mechanism with coroutine support.
 *          This lock ensures that only one coroutine can enter the critical
 *          section at a time, with automatic queuing of waiting coroutines.
 *
 * @note    This is specifically designed for async/await patterns in the
 *          FB game server where blocking operations must be avoided.
 */
class async_lock
{
public:
    /**
     * @brief   Constructs async_lock with initial semaphore count
     *
     * @param[in] initial Initial semaphore count (default: 1 for exclusive lock)
     */
    explicit async_lock(int initial = 1) :
        _count(initial)
    { }

    /**
     * @brief   Enters critical section and executes function asynchronously
     *
     *          Acquires lock, executes the provided function, and releases
     *          lock automatically with RAII-style exception safety.
     *
     * @tparam     T      Return type of the function
     * @param[in]  func   Async function to execute in critical section
     * @return     Result of the executed function
     *
     * @note    Thread-safe and exception-safe
     */
    template <typename T>
    async::task<T> enter(std::function<async::task<T>()> func)
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

    /**
     * @brief   Acquires the lock asynchronously
     *
     * @return  Task that completes when lock is acquired
     */
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

    /**
     * @brief   Releases the lock and notifies waiting coroutines
     */
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

/**
 * @brief   Asynchronous shared mutex for reader-writer synchronization
 *
 *          Provides shared (read) and exclusive (write) access control
 *          for asynchronous operations with coroutine support.
 */
class async_shared_mutex
{
public:
    /**
     * @brief   Acquires shared (read) lock asynchronously
     *
     * @return  Task that completes when shared lock is acquired
     */
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

    /**
     * @brief   Releases shared (read) lock
     */
    void unlock_shared()
    {
        if (this->_reader_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
            this->notify_writer();
    }

    /**
     * @brief   Acquires exclusive (write) lock asynchronously
     *
     * @return  Task that completes when exclusive lock is acquired
     */
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

    /**
     * @brief   Releases exclusive (write) lock
     */
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

private:
    std::atomic<int>                                                 _reader_count{0};
    std::atomic<bool>                                                _writer{false};
    mutable std::mutex                                               _mutex;
    std::queue<std::shared_ptr<async::task_completion_source<void>>> _reader_waiters;
    std::queue<std::shared_ptr<async::task_completion_source<void>>> _writer_waiters;

    /**
     * @brief   Checks if writer queue is empty
     *
     * @return  true if no writers are waiting, false otherwise
     */
    bool writer_queue_empty() const
    {
        std::lock_guard lk(this->_mutex);
        return this->_writer_waiters.empty();
    }

    /**
     * @brief   Notifies waiting writer if available
     */
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

/**
 * @brief      A thread-safe wrapper that provides synchronized access to a value.
 *
 *             This template class wraps a value of type ValueType and provides
 *             thread-safe read and write access using a shared_mutex. It supports
 *             multiple concurrent readers or a single writer, following the
 *             readers-writer lock pattern.
 *
 * @tparam     ValueType  The type of value to protect with synchronization.
 */
template <typename ValueType>
class locker
{
private:
    mutable std::shared_mutex  _sync_mutex;
    mutable async_shared_mutex _async_mutex;
    ValueType                  _value;

public:
    /**
     * @brief      Constructs a locker with the given arguments forwarded to ValueType.
     *
     *             Perfect forwarding constructor that allows initialization of the
     *             wrapped value with any arguments that ValueType's constructor accepts.
     *
     * @param[in]  args  Arguments to forward to ValueType's constructor.
     *
     * @tparam     Args  Parameter pack for constructor arguments.
     */
    template <typename... Args>
    locker(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    locker(const locker&) = delete;

    /**
     * @brief      Move constructor is deleted to prevent moving.
     */
    locker(locker&&) = delete;

    /**
     * @brief      Destructor is defaulted.
     */
    ~locker() = default;

    /**
     * @brief      Performs a write operation with exclusive access.
     *
     *             Acquires an exclusive lock and executes the provided function
     *             with write access to the wrapped value. Only one thread can
     *             perform write operations at a time.
     *
     * @param[in]  fn    Function to execute with write access to the value.
     *
     * @tparam     Func  The function type (lambda or function object).
     *
     * @return     The value returned by the function.
     */
    template <typename Func>
    auto write(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
    {
        auto _ = std::unique_lock(this->_sync_mutex);
        return fn(this->_value);
    }

    /**
     * @brief      Performs a read operation with shared access.
     *
     *             Acquires a shared lock and executes the provided function
     *             with read-only access to the wrapped value. Multiple threads
     *             can perform read operations concurrently.
     *
     * @param[in]  fn    Function to execute with read access to the value.
     *
     * @tparam     Func  The function type (lambda or function object).
     *
     * @return     The value returned by the function.
     */
    template <typename Func>
    auto read(Func&& fn) const -> decltype(fn(std::declval<const ValueType&>()))
    {
        auto _ = std::shared_lock(this->_sync_mutex);
        return fn(this->_value);
    }

    /**
     * @brief      Performs an asynchronous read operation with shared access.
     *
     *             Acquires a shared lock asynchronously and executes the provided function
     *             with read-only access to the wrapped value. Multiple coroutines
     *             can perform read operations concurrently.
     *
     * @param[in]  fn    Async function to execute with read access to the value.
     *
     * @tparam     Func  The function type (lambda or function object).
     *
     * @return     Task that completes with the value returned by the function.
     */
    template <typename Func>
    auto async_read(Func&& fn) const -> decltype(fn(std::declval<const ValueType&>()))
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

    /**
     * @brief      Performs an asynchronous write operation with exclusive access.
     *
     *             Acquires an exclusive lock asynchronously and executes the provided function
     *             with write access to the wrapped value. Only one coroutine can
     *             perform write operations at a time.
     *
     * @param[in]  fn    Async function to execute with write access to the value.
     *
     * @tparam     Func  The function type (lambda or function object).
     *
     * @return     Task that completes with the value returned by the function.
     */
    template <typename Func>
    auto async_write(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
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
};

/**
 * @brief      A simple thread-safe wrapper with recursive locking support.
 *
 *             This template class wraps a value of type ValueType and provides
 *             thread-safe access with support for recursive locking from the same thread.
 *             Uses std::recursive_mutex for simple and reliable recursive locking.
 *
 * @tparam     ValueType  The type of value to protect with synchronization.
 */
template <typename ValueType>
class recursive_locker
{
private:
    mutable std::recursive_mutex _mutex;
    ValueType                    _value;

public:
    /**
     * @brief      Constructs a recursive_locker with the given arguments forwarded to ValueType.
     *
     * @param[in]  args  Arguments to forward to ValueType's constructor.
     *
     * @tparam     Args  Parameter pack for constructor arguments.
     */
    template <typename... Args>
    recursive_locker(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    recursive_locker(const recursive_locker&) = delete;

    /**
     * @brief      Move constructor is deleted to prevent moving.
     */
    recursive_locker(recursive_locker&&) = delete;

    /**
     * @brief      Destructor is defaulted.
     */
    ~recursive_locker() = default;

    /**
     * @brief      Performs an operation with recursive lock access.
     *
     *             Acquires a recursive lock and executes the provided function
     *             with access to the wrapped value. The same thread can acquire
     *             the lock multiple times without deadlocking.
     *
     * @param[in]  fn    Function to execute with access to the value.
     *
     * @tparam     Func  The function type (lambda or function object).
     *
     * @return     The value returned by the function.
     */
    template <typename Func>
    auto lock(Func&& fn) -> decltype(fn(std::declval<ValueType&>()))
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

    // Backward compatibility with existing locker interface
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
