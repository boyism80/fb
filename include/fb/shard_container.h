#ifndef FB_SHARD_CONTAINER_H
#define FB_SHARD_CONTAINER_H

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

    explicit unordered_lru_map(size_t capacity = 0xFFFFFFFF) :
        _capacity(capacity)
    { }

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

    bool emplace(HashType hash, T&& value)
    {
        if (this->contains(hash))
            return false;
        this->super::emplace(hash, std::move(value));
        this->_lru_list.push_front(hash);
        return true;
    }

    T& operator[] (HashType hash)
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::operator[] (hash);
    }

    const T& operator[] (HashType hash) const
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::at(hash);
    }

    T& at(HashType hash)
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::at(hash);
    }

    const T& at(HashType hash) const
    {
        // Update LRU list
        auto it = std::find(this->_lru_list.begin(), this->_lru_list.end(), hash);
        if (it != this->_lru_list.end())
            this->_lru_list.erase(it);
        this->_lru_list.push_front(hash);
        return this->super::at(hash);
    }

    bool erase(HashType hash)
    {
        if (!this->contains(hash))
            return false;

        this->super::erase(hash);

        // Remove from LRU list
        this->_lru_list.remove(hash);

        return true;
    }

    size_t size() const
    {
        return this->std::unordered_map<HashType, T>::size();
    }

    bool contains(HashType hash) const
    {
        return this->std::unordered_map<HashType, T>::contains(hash);
    }

    std::vector<HashType> keys() const
    {
        std::vector<HashType> out;
        out.reserve(this->super::size());
        for (auto it = this->super::begin(); it != this->super::end(); ++it)
        {
            out.push_back(it->first);
        }
        return out;
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
    explicit sub_container(size_t capacity = 0xFFFFFFFF) :
        _data(map_type(capacity))
    { }

    bool insert(HashType hash, const T& value)
    {
        return this->_data.write([&](map_type& data) {
            if (data.contains(hash))
                return false;

            data[hash] = value;
            return true;
        });
    }

    template <typename Callback = std::nullptr_t>
    bool erase(HashType hash, Callback&& callback = nullptr)
    {
        if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>)
        {
            // No callback version
            return this->_data.write([&](map_type& data) {
                if (!data.contains(hash))
                    return false;

                data.erase(hash);
                return true;
            });
        }
        else
        {
            // With callback version - callback receives the element before erasure
            // Callback signature: (const T&) -> void
            auto callback_holder = std::make_shared<std::decay_t<Callback>>(std::forward<Callback>(callback));

            return this->_data.write([hash, callback_holder](map_type& data) {
                if (!data.contains(hash))
                    return false;

                // Get reference to element before erasure
                const auto& element = data.at(hash);

                // Call callback with element before erasing
                (*callback_holder)(element);

                // Erase after callback
                data.erase(hash);
                return true;
            });
        }
    }

    template <typename Callback = std::nullptr_t>
    auto async_erase(HashType hash, Callback&& callback = nullptr)
    {
        if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>)
        {
            // No callback version
            return this->_data.async_write([hash](map_type& data) -> async::task<bool> {
                if (!data.contains(hash))
                    co_return false;

                data.erase(hash);
                co_return true;
            });
        }
        else
        {
            // With callback version - callback receives the element before erasure
            // Callback signature: (const T&) -> async::task<void> or void
            using callback_result = decltype(callback(std::declval<const T&>()));
            using task_type       = std::conditional_t<std::is_same_v<callback_result, async::task<void>>,
                                                       async::task<void>,
                                                       async::task<callback_result>>;

            auto callback_holder = std::make_shared<std::decay_t<Callback>>(std::forward<Callback>(callback));

            if constexpr (std::is_same_v<callback_result, async::task<void>>)
            {
                return this->_data.async_write([hash, callback_holder](map_type& data) mutable -> async::task<void> {
                    if (!data.contains(hash))
                        co_return;

                    // Get reference to element before erasure
                    const auto& element = data.at(hash);

                    // Call callback with element before erasing
                    co_await (*callback_holder)(element);

                    // Erase after callback
                    data.erase(hash);
                });
            }
            else if constexpr (std::is_same_v<callback_result, void>)
            {
                return this->_data.async_write([hash, callback_holder](map_type& data) mutable -> async::task<void> {
                    if (!data.contains(hash))
                        co_return;

                    // Get reference to element before erasure
                    const auto& element = data.at(hash);

                    // Call callback with element before erasing
                    (*callback_holder)(element);

                    // Erase after callback
                    data.erase(hash);
                });
            }
            else
            {
                return this->_data.async_write([hash, callback_holder](map_type& data) mutable -> task_type {
                    if (!data.contains(hash))
                        co_return callback_result{};

                    // Get reference to element before erasure
                    const auto& element = data.at(hash);

                    // Call callback with element before erasing
                    auto result = co_await (*callback_holder)(element);

                    // Erase after callback
                    data.erase(hash);

                    co_return result;
                });
            }
        }
    }

    template <typename Func>
    auto read(HashType hash, Func&& func) const
    {
        return this->_data.read([&](const map_type& data) {
            if (!data.contains(hash))
                throw std::runtime_error("Element not found");

            return func(data.at(hash));
        });
    }

    template <typename Func>
    bool try_read(HashType hash, Func&& func) const
    {
        return this->_data.read([&](const map_type& data) {
            if (!data.contains(hash))
                return false;
            func(data.at(hash));
            return true;
        });
    }

    template <typename Func>
    auto write(HashType hash, Func&& func)
    {
        return this->_data.write([&](map_type& data) {
            if (!data.contains(hash))
                throw std::runtime_error("Element not found");

            return func(data.at(hash));
        });
    }

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

    template <typename Func>
    auto try_async_read(HashType hash, Func&& func) const -> async::task<bool>
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));

        return this->_data.try_async_read([hash, func_holder](const map_type& data) mutable -> async::task<void> {
            if (!data.contains(hash))
                throw std::runtime_error("Element not found");

            co_await (*func_holder)(data.at(hash));
        });
    }

    template <typename Func>
    auto try_async_write(HashType hash, Func&& func) -> async::task<bool>
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));

        return this->_data.try_async_write([hash, func_holder](map_type& data) mutable -> async::task<void> {
            if (!data.contains(hash))
                throw std::runtime_error("Element not found");

            co_await (*func_holder)(data.at(hash));
        });
    }

    std::vector<HashType> keys() const
    {
        return this->_data.read([&](const map_type& m) {
            return m.keys();
        });
    }

    template <typename Func, typename Factory>
    auto try_async_write(HashType hash, Func&& func, Factory&& factory) -> async::task<bool>
    {
        using task_type = decltype(func(std::declval<T&>()));

        auto func_holder    = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));
        auto factory_holder = std::make_shared<std::decay_t<Factory>>(std::forward<Factory>(factory));

        if constexpr (std::is_same_v<task_type, async::task<void>>)
        {
            return this->_data.try_async_write(
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
            return this->_data.try_async_write(
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
    }

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

    bool insert(const T& value)
    {
        auto hash = this->_hash_func(value);
        return this->bucket(hash)->insert(hash, value);
    }

    template <typename Callback = std::nullptr_t>
    bool erase(HashType hash, Callback&& callback = nullptr)
    {
        return this->bucket(hash)->erase(hash, std::forward<Callback>(callback));
    }

    template <typename Callback = std::nullptr_t>
    auto async_erase(HashType hash, Callback&& callback = nullptr)
    {
        return this->bucket(hash)->async_erase(hash, std::forward<Callback>(callback));
    }

    template <typename Func>
    auto read(HashType hash, Func&& func) const
    {
        return this->bucket(hash)->read(hash, std::forward<Func>(func));
    }

    template <typename Func>
    bool try_read(HashType hash, Func&& func) const
    {
        return this->bucket(hash)->try_read(hash, std::forward<Func>(func));
    }

    template <typename Func>
    auto write(HashType hash, Func&& func)
    {
        return this->bucket(hash)->write(hash, std::forward<Func>(func));
    }

    template <typename Func, typename Factory>
    auto write(HashType hash, Func&& func, Factory&& factory)
    {
        return this->bucket(hash)->write(hash, std::forward<Func>(func), std::forward<Factory>(factory));
    }

    template <typename Func>
    auto async_read(HashType hash, Func&& func) const -> decltype(func(std::declval<const T&>()))
    {
        return this->bucket(hash)->async_read(hash, std::forward<Func>(func));
    }

    template <typename Func>
    auto async_write(HashType hash, Func&& func)
    {
        return this->bucket(hash)->async_write(hash, std::forward<Func>(func));
    }

    template <typename Func, typename Factory>
    auto async_write(HashType hash, Func&& func, Factory&& factory)
    {
        return this->bucket(hash)->async_write(hash, std::forward<Func>(func), std::forward<Factory>(factory));
    }

    template <typename Func>
    auto try_async_read(HashType hash, Func&& func) const -> async::task<bool>
    {
        return this->bucket(hash)->try_async_read(hash, std::forward<Func>(func));
    }

    template <typename Func>
    auto try_async_write(HashType hash, Func&& func) -> async::task<bool>
    {
        return this->bucket(hash)->try_async_write(hash, std::forward<Func>(func));
    }

    template <typename Func, typename Factory>
    auto try_async_write(HashType hash, Func&& func, Factory&& factory) -> async::task<bool>
    {
        return this->bucket(hash)->try_async_write(hash, std::forward<Func>(func), std::forward<Factory>(factory));
    }

    std::vector<HashType> keys() const
    {
        std::vector<HashType> out;
        for (const auto& b : _buckets)
        {
            auto k = b->keys();
            for (auto h : k)
            {
                out.push_back(h);
            }
        }
        return out;
    }

private:
    static constexpr size_t bucket_index(HashType hash) noexcept
    {
        return hash % NumBuckets;
    }

    sub_container_type* bucket(HashType hash)
    {
        return this->_buckets[bucket_index(hash)].get();
    }

    const sub_container_type* bucket(HashType hash) const
    {
        return this->_buckets[bucket_index(hash)].get();
    }
};
} // namespace fb

#endif // FB_SHARD_CONTAINER_H
