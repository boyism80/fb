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
#include <list>
#include <fb/locker.h>

// Microsoft cpp-async library
#include <async/task.h>
#include <async/task_completion_source.h>

namespace fb {
template <typename T, typename HashType = uint32_t>
class unordered_lru_map : private std::unordered_map<HashType, T>
{
public:
    using super = std::unordered_map<HashType, T>;

    /**
     * @brief   Constructor with capacity
     *
     * @param[in] capacity Maximum number of elements (default: 0xFFFFFFFF)
     */
    explicit unordered_lru_map(size_t capacity = 0xFFFFFFFF) :
        _capacity(capacity)
    { }

    /**
     * @brief   Inserts element with given hash
     *
     * @param[in] hash Hash value for the element
     * @param[in] value Element to insert
     * @return    true if element was inserted, false if already exists
     */
    bool insert(HashType hash, const T& value)
    {
        if (this->contains(hash))
            return false;

        // Check capacity and remove LRU if necessary
        if (this->size() >= this->_capacity)
        {
            if (!this->_lru_list.empty())
            {
                HashType lru_hash = this->_lru_list.back();
                this->_lru_list.pop_back();
                this->erase(lru_hash);
            }
        }

        this->super::insert({hash, value});

        // Update LRU list
        // Remove from current position if exists
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
        {
            this->_lru_list.erase(it);
        }
        // Add to front (most recently used)
        this->_lru_list.push_front(hash);

        return true;
    }

    /**
     * @brief   Inserts element with given hash (does not check for duplicates)
     *
     * @param[in] hash Hash value for the element
     * @param[in] value Element to insert (rvalue)
     * @return    true if element was inserted, false if already exists
     */
    bool emplace(HashType hash, T&& value)
    {
        if (this->contains(hash))
            return false;
        this->super::emplace(hash, std::move(value));
        this->_lru_list.push_front(hash);
        return true;
    }

    /**
     * @brief   Access element and update LRU (non-const)
     *
     * @param[in] hash Hash of the target element
     * @return    Reference to the element
     */
    T& operator[] (HashType hash)
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::operator[] (hash);
    }

    /**
     * @brief   Access element and update LRU (const)
     *
     * @param[in] hash Hash of the target element
     * @return    Const reference to the element
     */
    const T& operator[] (HashType hash) const
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::at(hash);
    }

    /**
     * @brief   Get element at hash (non-const)
     *
     * @param[in] hash Hash of the target element
     * @return    Reference to the element
     */
    T& at(HashType hash)
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::at(hash);
    }

    /**
     * @brief   Get element at hash (const)
     *
     * @param[in] hash Hash of the target element
     * @return    Const reference to the element
     */
    const T& at(HashType hash) const
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::at(hash);
    }

    /**
     * @brief   Removes element by its hash
     *
     * @param[in] hash Hash of the target element
     * @return    true if element was removed, false if not found
     */
    bool erase(HashType hash)
    {
        if (!this->contains(hash))
            return false;

        this->super::erase(hash);

        // Remove from LRU list
        this->_lru_list.remove(hash);

        return true;
    }

    /**
     * @brief   Get size of the map
     *
     * @return    Number of elements
     */
    size_t size() const
    {
        return this->std::unordered_map<HashType, T>::size();
    }

    /**
     * @brief   Check if element exists
     *
     * @param[in] hash Hash of the target element
     * @return    true if element exists, false otherwise
     */
    bool contains(HashType hash) const
    {
        return this->std::unordered_map<HashType, T>::contains(hash);
    }

private:
    mutable std::list<HashType> _lru_list; ///< LRU list to track access order
    size_t                      _capacity; ///< Maximum number of elements
};

template <typename T, typename HashType = uint32_t>
class sub_container
{
public:
    using map_type = unordered_lru_map<T, HashType>;

private:
    fb::locker<map_type> _data;

public:
    /**
     * @brief   Constructor with capacity
     *
     * @param[in] capacity Maximum number of elements (default: 0xFFFFFFFF)
     */
    explicit sub_container(size_t capacity = 0xFFFFFFFF) :
        _data(map_type(capacity))
    { }

    /**
     * @brief   Inserts element with given hash
     *
     * @param[in] hash Hash value for the element
     * @param[in] value Element to insert
     * @return    true if element was inserted, false if already exists
     */
    bool insert(HashType hash, const T& value)
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
    bool erase(HashType hash)
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
    auto read(HashType hash, Func&& func) const
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
    auto write(HashType hash, Func&& func)
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
    auto write(HashType hash, Func&& func, Factory&& factory)
    {
        return this->_data.write([&](map_type& data) {
            if (!data.contains(hash))
            {
                // Create element using factory
                if constexpr (std::is_invocable_r_v<T, Factory>)
                {
                    // Regular function returning T
                    data.insert(hash, factory());
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
    auto async_read(HashType hash, Func&& func) const -> decltype(func(std::declval<const T&>()))
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
    auto async_write(HashType hash, Func&& func)
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
    auto async_write(HashType hash, Func&& func, Factory&& factory)
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
                            data.insert(hash, (*factory_holder)());
                        }
                        else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                        {
                            // Coroutine function returning async::task<T>
                            data.insert(hash, co_await (*factory_holder)());
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
                        data.insert(hash, (*factory_holder)());
                    }
                    else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                    {
                        // Coroutine function returning async::task<T>
                        data.insert(hash, co_await (*factory_holder)());
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
 * @tparam HashType    Type of hash key (default: uint32_t)
 */
template <typename T, size_t NumBuckets, typename HashType = uint32_t>
class sharded_container
{
    static_assert(NumBuckets > 0, "NumBuckets must be positive");

public:
    using hash_function_type = std::function<HashType(const T&)>;
    using sub_container_type = sub_container<T, HashType>;
    using bucket_type        = std::unique_ptr<sub_container_type>;
    using buckets_type       = std::array<bucket_type, NumBuckets>;

private:
    buckets_type       _buckets;
    hash_function_type _hash_func;

public:
    /**
     * @brief   Constructor with hash function and capacity
     *
     * @param[in] hash_func Function to calculate hash for elements
     * @param[in] capacity  Capacity per bucket (default: 0xFFFFFFFF)
     */
    explicit sharded_container(hash_function_type hash_func, size_t capacity = 0xFFFFFFFF) :
        _hash_func(std::move(hash_func))
    {
        // Initialize buckets with capacity
        for (auto& bucket : _buckets)
        {
            bucket = std::make_unique<sub_container_type>(capacity);
        }
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
        return this->bucket(hash)->insert(hash, value);
    }

    /**
     * @brief   Removes element using hash function
     *
     * @param[in] hash Hash of the element to remove
     */
    bool erase(HashType hash)
    {
        return this->bucket(hash)->erase(hash);
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
    auto read(HashType hash, Func&& func) const
    {
        return this->bucket(hash)->read(hash, std::forward<Func>(func));
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
    auto write(HashType hash, Func&& func)
    {
        return this->bucket(hash)->write(hash, std::forward<Func>(func));
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
    auto write(HashType hash, Func&& func, Factory&& factory)
    {
        return this->bucket(hash)->write(hash, std::forward<Func>(func), std::forward<Factory>(factory));
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
    auto async_read(HashType hash, Func&& func) const -> decltype(func(std::declval<const T&>()))
    {
        return this->bucket(hash)->async_read(hash, std::forward<Func>(func));
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
    auto async_write(HashType hash, Func&& func)
    {
        return this->bucket(hash)->async_write(hash, std::forward<Func>(func));
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
    auto async_write(HashType hash, Func&& func, Factory&& factory)
    {
        return this->bucket(hash)->async_write(hash, std::forward<Func>(func), std::forward<Factory>(factory));
    }

private:
    /**
     * @brief   Calculates bucket index from hash
     *
     * @param[in] hash Hash value
     * @return    Bucket index
     */
    static constexpr size_t bucket_index(HashType hash) noexcept
    {
        return hash % NumBuckets;
    }

    /**
     * @brief   Gets mutable pointer to bucket by hash
     *
     * @param[in] hash Hash value
     * @return    Pointer to the target bucket
     */
    sub_container_type* bucket(HashType hash)
    {
        return this->_buckets[bucket_index(hash)].get();
    }

    /**
     * @brief   Gets const pointer to bucket by hash
     *
     * @param[in] hash Hash value
     * @return    Const pointer to the target bucket
     */
    const sub_container_type* bucket(HashType hash) const
    {
        return this->_buckets[bucket_index(hash)].get();
    }
};
} // namespace fb

#endif // FB_SHARD_CONTAINER_H
