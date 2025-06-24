#ifndef FB_SHARD_CONTAINER_H
#define FB_SHARD_CONTAINER_H

/**
 * @file    shard_container.h
 * @brief   Thread-safe sharded container with asynchronous operations
 * @author  FB Development Team
 *
 * @details This file implements a high-performance sharded container system
 *          designed for concurrent access in the FB 2D MMORPG game server.
 *          The container distributes data across multiple sub-containers
 *          (shards) to reduce lock contention and improve performance.
 *
 *          Key features:
 *          - Hash-based data distribution across multiple shards
 *          - Async/await coroutine support for non-blocking operations
 *          - Both synchronous and asynchronous access patterns
 *          - Template-based automatic type deduction
 *          - Factory function support for lazy element creation
 *          - Thread-safe operations with minimal lock contention
 *
 * @note    This container is specifically designed for the FB game server
 *          architecture where high concurrency and low latency are critical.
 */

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <unordered_map>
#include <vector>
#include <array>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <cstdint>
#include <memory>
#include <coroutine>

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
 * @brief   Sub-container for sharded storage with synchronization
 *
 *          Provides both synchronous and asynchronous access to a hash map
 *          with proper reader-writer synchronization for concurrent access.
 *
 * @tparam T Type stored in container
 */
template <typename T>
class sub_container
{
public:
    using map_type = std::unordered_map<uint32_t, T>;

private:
    map_type                   _data;
    mutable std::shared_mutex  _sync_mutex;
    mutable async_shared_mutex _async_mutex;

public:
    /**
     * @brief   Default constructor
     */
    sub_container() = default;

    /**
     * @brief   Inserts element with given hash
     *
     * @param[in] hash Hash value for the element
     * @param[in] value Element to insert
     * @return    true if element was inserted, false if already exists
     */
    bool insert(uint32_t hash, const T& value)
    {
        if (this->_data.contains(hash))
            return false;

        this->_data[hash] = value;
        return true;
    }

    /**
     * @brief   Removes element by its hash
     *
     * @param[in] hash Hash of the target element
     * @return    true if element was removed, false if not found
     */
    bool erase(uint32_t hash)
    {
        if (!this->_data.contains(hash))
            return false;

        this->_data.erase(hash);
        return true;
    }

    /**
     * @brief   Performs synchronous read operation
     *
     * @tparam     Func Function type for read operation
     * @param[in]  func Function to execute with read access
     * @return     Result of the function execution
     */
    template <typename Func>
    auto read(uint32_t hash, Func&& func) const
    {
        std::shared_lock lock(this->_sync_mutex);
        if (!this->_data.contains(hash))
            throw std::runtime_error("Element not found");

        return func(this->_data.at(hash));
    }

    /**
     * @brief   Performs synchronous write operation
     *
     * @tparam     Func Function type for write operation
     * @param[in]  func Function to execute with write access
     * @return     Result of the function execution
     */
    template <typename Func>
    auto write(uint32_t hash, Func&& func)
    {
        std::unique_lock lock(this->_sync_mutex);
        if (!this->_data.contains(hash))
            throw std::runtime_error("Element not found");

        return func(this->_data.at(hash));
    }

    /**
     * @brief   Performs synchronous write operation with factory
     *
     * @tparam     Func Function type for write operation
     * @tparam     Factory Function type for element creation
     * @param[in]  hash Hash of the target element
     * @param[in]  func Function to execute with write access
     * @param[in]  factory Function to create element if not found
     * @return     Result of the function execution
     */
    template <typename Func, typename Factory>
    auto write(uint32_t hash, Func&& func, Factory&& factory)
    {
        std::unique_lock lock(this->_sync_mutex);

        if (!this->_data.contains(hash))
        {
            // Create element using factory
            if constexpr (std::is_invocable_r_v<T, Factory>)
            {
                // Regular function returning T
                this->_data.emplace(hash, factory());
            }
            else
            {
                static_assert(std::is_invocable_r_v<T, Factory>, "Factory must return T");
            }
        }

        return func(this->_data.at(hash));
    }

    /**
     * @brief   Performs asynchronous read operation with automatic return type detection
     *
     * @tparam     Func Function type for async read operation
     * @param[in]  hash Hash of the target element
     * @param[in]  func Async function to execute with read access to the element
     * @return     Task that completes when operation finishes (void) or contains result
     */
    template <typename Func>
    auto async_read(uint32_t hash, Func&& func) const -> decltype(func(std::declval<const T&>()))
    {
        using task_type = decltype(func(std::declval<const T&>()));

        co_await this->_async_mutex.lock_shared();
        std::shared_lock lock(this->_sync_mutex);

        try
        {
            if (!this->_data.contains(hash))
            {
                this->_async_mutex.unlock_shared();
                throw std::runtime_error("Element not found");
            }

            if constexpr (std::is_same_v<task_type, async::task<void>>)
            {
                co_await func(this->_data.at(hash));
                this->_async_mutex.unlock_shared();
                co_return;
            }
            else
            {
                auto result = co_await func(this->_data.at(hash));
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
     * @brief   Performs asynchronous write operation with automatic return type detection
     *
     * @tparam     Func Function type for async write operation
     * @param[in]  hash Hash of the target element
     * @param[in]  func Async function to execute with write access
     * @return     Task that completes when operation finishes (void) or contains result
     */
    template <typename Func>
    auto async_write(uint32_t hash, Func&& func)
    {
        return this->async_write_impl(hash, std::forward<Func>(func));
    }

    /**
     * @brief   Performs asynchronous write operation with factory and automatic return type detection
     *
     * @tparam     Func Function type for async write operation
     * @tparam     Factory Function type for element creation
     * @param[in]  hash Hash of the target element
     * @param[in]  func Async function to execute with write access
     * @param[in]  factory Function to create element if not found
     * @return     Task that completes when operation finishes (void) or contains result
     */
    template <typename Func, typename Factory>
    auto async_write(uint32_t hash, Func&& func, Factory&& factory)
    {
        return this->async_write_factory_impl(hash, std::forward<Func>(func), std::forward<Factory>(factory));
    }

private:
    template <typename Func>
    auto async_write_impl(uint32_t hash, Func&& func)
        -> std::enable_if_t<std::is_same_v<decltype(func(std::declval<T&>())), async::task<void>>, async::task<void>>
    {
        co_await this->_async_mutex.lock();
        std::unique_lock lock(this->_sync_mutex);

        try
        {
            if (!this->_data.contains(hash))
            {
                this->_async_mutex.unlock();
                throw std::runtime_error("Element not found");
            }

            co_await func(this->_data.at(hash));
            this->_async_mutex.unlock();
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }

    template <typename Func>
    auto async_write_impl(uint32_t hash, Func&& func)
        -> std::enable_if_t<!std::is_same_v<decltype(func(std::declval<T&>())), async::task<void>>,
                            decltype(func(std::declval<T&>()))>
    {
        co_await this->_async_mutex.lock();
        std::unique_lock lock(this->_sync_mutex);

        try
        {
            if (!this->_data.contains(hash))
            {
                this->_async_mutex.unlock();
                throw std::runtime_error("Element not found");
            }

            auto result = co_await func(this->_data.at(hash));
            this->_async_mutex.unlock();
            co_return result;
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }

    template <typename Func, typename Factory>
    auto async_write_factory_impl(uint32_t hash, Func&& func, Factory&& factory)
        -> std::enable_if_t<std::is_same_v<decltype(func(std::declval<T&>())), async::task<void>>, async::task<void>>
    {
        co_await this->_async_mutex.lock();
        std::unique_lock lock(this->_sync_mutex);

        try
        {
            if (!this->_data.contains(hash))
            {
                // Try to create element using factory
                if constexpr (std::is_invocable_r_v<T, Factory>)
                {
                    // Regular function returning T
                    this->_data.emplace(hash, factory());
                }
                else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                {
                    // Coroutine function returning async::task<T>
                    this->_data.emplace(hash, co_await factory());
                }
                else
                {
                    static_assert(std::is_invocable_r_v<T, Factory> || std::is_invocable_r_v<async::task<T>, Factory>,
                                  "Factory must return T or async::task<T>");
                }
            }

            co_await func(this->_data.at(hash));
            this->_async_mutex.unlock();
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }

    template <typename Func, typename Factory>
    auto async_write_factory_impl(uint32_t hash, Func&& func, Factory&& factory)
        -> std::enable_if_t<!std::is_same_v<decltype(func(std::declval<T&>())), async::task<void>>,
                            decltype(func(std::declval<T&>()))>
    {
        co_await this->_async_mutex.lock();
        std::unique_lock lock(this->_sync_mutex);

        try
        {
            if (!this->_data.contains(hash))
            {
                // Try to create element using factory
                if constexpr (std::is_invocable_r_v<T, Factory>)
                {
                    // Regular function returning T
                    this->_data.emplace(hash, factory());
                }
                else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                {
                    // Coroutine function returning async::task<T>
                    this->_data.emplace(hash, co_await factory());
                }
                else
                {
                    static_assert(std::is_invocable_r_v<T, Factory> || std::is_invocable_r_v<async::task<T>, Factory>,
                                  "Factory must return T or async::task<T>");
                }
            }

            auto result = co_await func(this->_data.at(hash));
            this->_async_mutex.unlock();
            co_return result;
        }
        catch (...)
        {
            this->_async_mutex.unlock();
            throw;
        }
    }
};

/**
 * @brief   Sharded container for high-performance concurrent access
 *
 *          Distributes data across multiple sub-containers (shards) to reduce
 *          lock contention and improve concurrent access performance.
 *
 * @tparam T           Type stored in container
 * @tparam NumBuckets  Number of shards/buckets for distribution
 */
template <typename T, size_t NumBuckets>
class sharded_container
{
    static_assert(NumBuckets > 0, "NumBuckets must be positive");

public:
    using hash_function_type = std::function<uint32_t(const T&)>;

private:
    std::array<sub_container<T>, NumBuckets> _buckets;
    hash_function_type                       _hash_func;

public:
    /**
     * @brief   Constructor with hash function
     *
     * @param[in] hash_func Function to calculate hash for elements
     */
    explicit sharded_container(hash_function_type hash_func) :
        _hash_func(std::move(hash_func))
    {
        // Buckets are default-constructed, no need to initialize
    }
    sharded_container(const sharded_container&) = delete;
    sharded_container(sharded_container&&)      = delete;

    /**
     * @brief   Inserts element using hash function
     *
     * @param[in] value Element to insert
     */
    bool insert(const T& value)
    {
        auto hash = this->_hash_func(value);
        return this->bucket(hash).insert(hash, value);
    }

    /**
     * @brief   Removes element using hash function
     *
     * @param[in] value Element to remove
     */
    bool erase(uint16_t hash)
    {
        return this->bucket(hash).erase(hash);
    }

    /**
     * @brief   Performs synchronous read operation by hash
     *
     * @tparam     Func Function type for read operation
     * @param[in]  hash Hash of the target shard
     * @param[in]  func Function to execute with read access
     * @return     Result of the function execution
     */
    template <typename Func>
    auto read(uint32_t hash, Func&& func) const
    {
        return this->bucket(hash).read(hash, std::forward<Func>(func));
    }

    /**
     * @brief   Performs synchronous write operation by hash
     *
     * @tparam     Func Function type for write operation
     * @param[in]  hash Hash of the target shard
     * @param[in]  func Function to execute with write access
     * @return     Result of the function execution
     */
    template <typename Func>
    auto write(uint32_t hash, Func&& func)
    {
        return this->bucket(hash).write(hash, std::forward<Func>(func));
    }

    /**
     * @brief   Performs synchronous write operation by hash with factory
     *
     * @tparam     Func Function type for write operation
     * @tparam     Factory Function type for element creation
     * @param[in]  hash Hash of the target shard
     * @param[in]  func Function to execute with write access
     * @param[in]  factory Function to create element if not found
     * @return     Result of the function execution
     */
    template <typename Func, typename Factory>
    auto write(uint32_t hash, Func&& func, Factory&& factory)
    {
        return this->bucket(hash).write(hash, std::forward<Func>(func), std::forward<Factory>(factory));
    }

    /**
     * @brief   Performs asynchronous read operation with automatic return type detection
     *
     * @tparam     Func Function type for async read operation
     * @param[in]  hash Hash of the target shard
     * @param[in]  func Async function to execute with read access
     * @return     Task that completes when operation finishes (void) or contains result
     */
    template <typename Func>
    auto async_read(uint32_t hash, Func&& func) const -> decltype(func(std::declval<const T&>()))
    {
        return this->bucket(hash).async_read(hash, std::forward<Func>(func));
    }

    /**
     * @brief   Performs asynchronous write operation with automatic return type detection
     *
     * @tparam     Func Function type for async write operation
     * @param[in]  hash Hash of the target shard
     * @param[in]  func Async function to execute with write access
     * @return     Task that completes when operation finishes (void) or contains result
     */
    template <typename Func>
    auto async_write(uint32_t hash, Func&& func)
    {
        return this->bucket(hash).async_write(hash, std::forward<Func>(func));
    }

    /**
     * @brief   Performs asynchronous write operation with factory and automatic return type detection
     *
     * @tparam     Func Function type for async write operation
     * @tparam     Factory Function type for element creation
     * @param[in]  hash Hash of the target element
     * @param[in]  func Async function to execute with write access
     * @param[in]  factory Function to create element if not found
     * @return     Task that completes when operation finishes (void) or contains result
     */
    template <typename Func, typename Factory>
    auto async_write(uint32_t hash, Func&& func, Factory&& factory)
    {
        return this->bucket(hash).async_write(hash, std::forward<Func>(func), std::forward<Factory>(factory));
    }

private:
    /**
     * @brief   Calculates bucket index from hash
     *
     * @param[in] hash Hash value
     * @return    Bucket index
     */
    static constexpr size_t bucket_index(uint32_t hash) noexcept
    {
        return hash % NumBuckets;
    }

    /**
     * @brief   Gets mutable reference to bucket by hash
     *
     * @param[in] hash Hash value
     * @return    Reference to the target bucket
     */
    sub_container<T>& bucket(uint32_t hash)
    {
        return this->_buckets[bucket_index(hash)];
    }

    /**
     * @brief   Gets const reference to bucket by hash
     *
     * @param[in] hash Hash value
     * @return    Const reference to the target bucket
     */
    const sub_container<T>& bucket(uint32_t hash) const
    {
        return this->_buckets[bucket_index(hash)];
    }
};
} // namespace fb

#endif // FB_SHARD_CONTAINER_H
