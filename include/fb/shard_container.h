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
#include <fb/locker.h>

// Microsoft cpp-async library
#include <async/task.h>
#include <async/task_completion_source.h>

namespace fb {
template <typename T>
class sub_container
{
public:
    using map_type = std::unordered_map<uint32_t, T>;

private:
    fb::locker<map_type> _data;

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
        return this->_data.write([&](map_type& data) {
            if (data.contains(hash))
                return false;

            data[hash] = value;
            return true;
        });
    }

    /**
     * @brief   Removes element by its hash
     *
     * @param[in] hash Hash of the target element
     * @return    true if element was removed, false if not found
     */
    bool erase(uint32_t hash)
    {
        return this->_data.write([&](map_type& data) {
            if (!data.contains(hash))
                return false;

            data.erase(hash);
            return true;
        });
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
        return this->_data.read([&](const map_type& data) {
            if (!data.contains(hash))
                throw std::runtime_error("Element not found");

            return func(data.at(hash));
        });
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
        return this->_data.write([&](map_type& data) {
            if (!data.contains(hash))
                throw std::runtime_error("Element not found");

            return func(data.at(hash));
        });
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
        return this->_data.write([&](map_type& data) {
            if (!data.contains(hash))
            {
                // Create element using factory
                if constexpr (std::is_invocable_r_v<T, Factory>)
                {
                    // Regular function returning T
                    data.emplace(hash, factory());
                }
                else
                {
                    static_assert(std::is_invocable_r_v<T, Factory>, "Factory must return T");
                }
            }

            return func(data.at(hash));
        });
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

        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));

        if constexpr (std::is_same_v<task_type, async::task<void>>)
        {
            co_return co_await this->_data.async_read(
                [hash, func_holder](const map_type& data) mutable -> async::task<void> {
                    if (!data.contains(hash))
                        throw std::runtime_error("Element not found");

                    co_await (*func_holder)(data.at(hash));
                });
        }
        else
        {
            co_return co_await this->_data.async_read([hash, func_holder](const map_type& data) mutable -> task_type {
                if (!data.contains(hash))
                    throw std::runtime_error("Element not found");

                co_return co_await (*func_holder)(data.at(hash));
            });
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
        using task_type = decltype(func(std::declval<T&>()));

        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));

        if constexpr (std::is_same_v<task_type, async::task<void>>)
        {
            return this->_data.async_write([hash, func_holder](map_type& data) mutable -> async::task<void> {
                if (!data.contains(hash))
                    throw std::runtime_error("Element not found");

                co_await (*func_holder)(data.at(hash));
            });
        }
        else
        {
            return this->_data.async_write([hash, func_holder](map_type& data) mutable -> task_type {
                if (!data.contains(hash))
                    throw std::runtime_error("Element not found");

                co_return co_await (*func_holder)(data.at(hash));
            });
        }
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
        using task_type = decltype(func(std::declval<T&>()));

        auto func_holder    = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));
        auto factory_holder = std::make_shared<std::decay_t<Factory>>(std::forward<Factory>(factory));

        if constexpr (std::is_same_v<task_type, async::task<void>>)
        {
            return this->_data.async_write(
                [hash, func_holder, factory_holder](map_type& data) mutable -> async::task<void> {
                    if (!data.contains(hash))
                    {
                        // Create element using factory
                        if constexpr (std::is_invocable_r_v<T, Factory>)
                        {
                            // Regular function returning T
                            data.emplace(hash, (*factory_holder)());
                        }
                        else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                        {
                            // Coroutine function returning async::task<T>
                            data.emplace(hash, co_await (*factory_holder)());
                        }
                        else
                        {
                            static_assert(std::is_invocable_r_v<T, Factory> ||
                                              std::is_invocable_r_v<async::task<T>, Factory>,
                                          "Factory must return T or async::task<T>");
                        }
                    }

                    co_await (*func_holder)(data.at(hash));
                });
        }
        else
        {
            return this->_data.async_write([hash, func_holder, factory_holder](map_type& data) mutable -> task_type {
                if (!data.contains(hash))
                {
                    // Create element using factory
                    if constexpr (std::is_invocable_r_v<T, Factory>)
                    {
                        // Regular function returning T
                        data.emplace(hash, (*factory_holder)());
                    }
                    else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                    {
                        // Coroutine function returning async::task<T>
                        data.emplace(hash, co_await (*factory_holder)());
                    }
                    else
                    {
                        static_assert(std::is_invocable_r_v<T, Factory> ||
                                          std::is_invocable_r_v<async::task<T>, Factory>,
                                      "Factory must return T or async::task<T>");
                    }
                }

                co_return co_await (*func_holder)(data.at(hash));
            });
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
