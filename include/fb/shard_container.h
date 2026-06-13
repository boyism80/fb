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
#include <stdexcept>
#include <fb/synchronized.h>

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

    class read_guard
    {
    public:
        read_guard(read_guard&&) noexcept         = default;
        read_guard& operator= (read_guard&&)      = delete;
        read_guard(const read_guard&)             = delete;
        read_guard& operator= (const read_guard&) = delete;

        const T& value() const noexcept
        {
            return this->_map_guard.value().at(this->_hash);
        }

    private:
        friend class sub_container;

        typename fb::synchronized<map_type>::read_guard _map_guard;
        HashType                                        _hash;

        read_guard(typename fb::synchronized<map_type>::read_guard&& map_guard, HashType hash) noexcept :
            _map_guard(std::move(map_guard)),
            _hash(hash)
        { }
    };

    class write_guard
    {
    public:
        write_guard(write_guard&&) noexcept         = default;
        write_guard& operator= (write_guard&&)      = delete;
        write_guard(const write_guard&)             = delete;
        write_guard& operator= (const write_guard&) = delete;

        T& value() noexcept
        {
            return this->_map_guard.value().at(this->_hash);
        }

    private:
        friend class sub_container;

        typename fb::synchronized<map_type>::write_guard _map_guard;
        HashType                                         _hash;

        write_guard(typename fb::synchronized<map_type>::write_guard&& map_guard, HashType hash) noexcept :
            _map_guard(std::move(map_guard)),
            _hash(hash)
        { }
    };

    class async_read_guard
    {
    public:
        async_read_guard(async_read_guard&&) noexcept         = default;
        async_read_guard& operator= (async_read_guard&&)      = delete;
        async_read_guard(const async_read_guard&)             = delete;
        async_read_guard& operator= (const async_read_guard&) = delete;

        const T& value() const noexcept
        {
            return this->_map_guard.value().at(this->_hash);
        }

    private:
        friend class sub_container;

        typename fb::synchronized<map_type>::async_read_guard _map_guard;
        HashType                                              _hash;

        async_read_guard(typename fb::synchronized<map_type>::async_read_guard&& map_guard, HashType hash) noexcept :
            _map_guard(std::move(map_guard)),
            _hash(hash)
        { }
    };

    class async_write_guard
    {
    public:
        async_write_guard(async_write_guard&&) noexcept         = default;
        async_write_guard& operator= (async_write_guard&&)      = delete;
        async_write_guard(const async_write_guard&)             = delete;
        async_write_guard& operator= (const async_write_guard&) = delete;

        T& value() noexcept
        {
            return this->_map_guard.value().at(this->_hash);
        }

    private:
        friend class sub_container;

        typename fb::synchronized<map_type>::async_write_guard _map_guard;
        HashType                                               _hash;

        async_write_guard(typename fb::synchronized<map_type>::async_write_guard&& map_guard, HashType hash) noexcept :
            _map_guard(std::move(map_guard)),
            _hash(hash)
        { }
    };

private:
    fb::synchronized<map_type> _data;

public:
    explicit sub_container(size_t capacity = 0xFFFFFFFF) :
        _data(map_type(capacity))
    { }

    [[nodiscard]] static bool needs_factory_insert(const map_type& data, HashType hash)
    {
        if (data.contains(hash) == false)
            return true;

        if constexpr (requires(const T& value) { value == nullptr; })
            return data.at(hash) == nullptr;

        return false;
    }

    bool insert(HashType hash, const T& value)
    {
        auto  guard = this->_data.enter_write();
        auto& data  = guard.value();
        if (data.contains(hash))
            return false;

        data[hash] = value;
        return true;
    }

    template <typename Callback = std::nullptr_t>
    bool erase(HashType hash, Callback&& callback = nullptr)
    {
        if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>)
        {
            // No callback version
            auto  guard = this->_data.enter_write();
            auto& data  = guard.value();
            if (!data.contains(hash))
                return false;

            data.erase(hash);
            return true;
        }
        else
        {
            // With callback version - callback receives the element before erasure
            // Callback signature: (const T&) -> void
            auto callback_holder = std::make_shared<std::decay_t<Callback>>(std::forward<Callback>(callback));

            auto  guard = this->_data.enter_write();
            auto& data  = guard.value();
            if (!data.contains(hash))
                return false;

            const auto& element = data.at(hash);
            (*callback_holder)(element);
            data.erase(hash);
            return true;
        }
    }

    template <typename Callback = std::nullptr_t>
    auto async_erase(HashType hash, Callback&& callback = nullptr)
    {
        if constexpr (std::is_same_v<std::decay_t<Callback>, std::nullptr_t>)
        {
            // No callback version
            return [this, hash]() -> async::task<bool> {
                auto  guard = co_await this->_data.enter_write_async();
                auto& data  = guard.value();
                if (!data.contains(hash))
                    co_return false;

                data.erase(hash);
                co_return true;
            }();
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

    read_guard enter_read(HashType hash) const
    {
        auto        map_guard = this->_data.enter_read();
        const auto& data      = map_guard.value();
        if (data.contains(hash) == false)
            throw std::runtime_error("Element not found");

        return read_guard(std::move(map_guard), hash);
    }

    std::optional<read_guard> try_enter_read(HashType hash) const
    {
        auto        map_guard = this->_data.enter_read();
        const auto& data      = map_guard.value();
        if (data.contains(hash) == false)
            return std::nullopt;

        return read_guard(std::move(map_guard), hash);
    }

    write_guard enter_write(HashType hash)
    {
        auto  map_guard = this->_data.enter_write();
        auto& data      = map_guard.value();
        if (data.contains(hash) == false)
            throw std::runtime_error("Element not found");

        return write_guard(std::move(map_guard), hash);
    }

    async::task<async_read_guard> enter_read_async(HashType hash) const
    {
        auto        map_guard = co_await this->_data.enter_read_async();
        const auto& data      = map_guard.value();
        if (data.contains(hash) == false)
            throw std::runtime_error("Element not found");

        co_return async_read_guard(std::move(map_guard), hash);
    }

    async::task<async_write_guard> enter_write_async(HashType hash)
    {
        auto  map_guard = co_await this->_data.enter_write_async();
        auto& data      = map_guard.value();
        if (data.contains(hash) == false)
            throw std::runtime_error("Element not found");

        co_return async_write_guard(std::move(map_guard), hash);
    }

    async::task<std::optional<async_read_guard>> try_enter_read_async(HashType hash) const
    {
        auto map_guard = co_await this->_data.try_enter_read_async();
        if (map_guard.has_value() == false)
            co_return std::nullopt;

        const auto& data = map_guard->value();
        if (data.contains(hash) == false)
            co_return std::nullopt;

        co_return async_read_guard(std::move(*map_guard), hash);
    }

    async::task<std::optional<async_write_guard>> try_enter_write_async(HashType hash)
    {
        auto map_guard = co_await this->_data.try_enter_write_async();
        if (map_guard.has_value() == false)
            co_return std::nullopt;

        auto& data = map_guard->value();
        if (data.contains(hash) == false)
            co_return std::nullopt;

        co_return async_write_guard(std::move(*map_guard), hash);
    }

    template <typename Func>
    auto read(HashType hash, Func&& func) const
    {
        auto guard = this->enter_read(hash);
        return func(guard.value());
    }

    template <typename Func>
    bool try_read(HashType hash, Func&& func) const
    {
        auto guard = this->try_enter_read(hash);
        if (guard.has_value() == false)
            return false;

        func(guard->value());
        return true;
    }

    template <typename Func>
    auto write(HashType hash, Func&& func)
    {
        auto guard = this->enter_write(hash);
        return func(guard.value());
    }

    template <typename Func, typename Factory>
    auto write(HashType hash, Func&& func, Factory&& factory)
    {
        auto  guard = this->_data.enter_write();
        auto& data  = guard.value();
        if (sub_container::needs_factory_insert(data, hash))
        {
            if constexpr (std::is_invocable_r_v<T, Factory>)
            {
                data[hash] = factory();
            }
            else
            {
                static_assert(std::is_invocable_r_v<T, Factory>, "Factory must return T");
            }
        }

        return func(data.at(hash));
    }

    template <typename Func>
    auto async_read(HashType hash, Func&& func) const -> decltype(func(std::declval<const T&>()))
    {
        using task_type = decltype(func(std::declval<const T&>()));

        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));
        auto guard       = co_await this->enter_read_async(hash);

        if constexpr (std::is_same_v<task_type, async::task<void>>)
            co_await (*func_holder)(guard.value());
        else
            co_return co_await (*func_holder)(guard.value());
    }

    template <typename Func>
    auto async_write(HashType hash, Func&& func)
    {
        using task_type = decltype(func(std::declval<T&>()));

        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));

        if constexpr (std::is_same_v<task_type, async::task<void>>)
        {
            return [this, hash, func_holder]() -> async::task<void> {
                auto guard = co_await this->enter_write_async(hash);
                co_await (*func_holder)(guard.value());
            }();
        }
        else
        {
            return [this, hash, func_holder]() -> task_type {
                auto guard = co_await this->enter_write_async(hash);
                co_return co_await (*func_holder)(guard.value());
            }();
        }
    }

    template <typename Func>
    auto try_async_read(HashType hash, Func&& func) const -> async::task<bool>
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));
        auto guard       = co_await this->try_enter_read_async(hash);

        if (guard.has_value() == false)
            co_return false;

        co_await (*func_holder)(guard->value());
        co_return true;
    }

    template <typename Func>
    auto try_async_write(HashType hash, Func&& func) -> async::task<bool>
    {
        auto func_holder = std::make_shared<std::decay_t<Func>>(std::forward<Func>(func));
        auto guard       = co_await this->try_enter_write_async(hash);

        if (guard.has_value() == false)
            co_return false;

        co_await (*func_holder)(guard->value());
        co_return true;
    }

    std::vector<HashType> keys() const
    {
        auto guard = this->_data.enter_read();
        return guard.value().keys();
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
                    if (sub_container::needs_factory_insert(data, hash))
                    {
                        if constexpr (std::is_invocable_r_v<T, Factory>)
                        {
                            data[hash] = (*factory_holder)();
                        }
                        else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                        {
                            data[hash] = co_await (*factory_holder)();
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
                    if (sub_container::needs_factory_insert(data, hash))
                    {
                        if constexpr (std::is_invocable_r_v<T, Factory>)
                        {
                            data[hash] = (*factory_holder)();
                        }
                        else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                        {
                            data[hash] = co_await (*factory_holder)();
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
                    if (sub_container::needs_factory_insert(data, hash))
                    {
                        if constexpr (std::is_invocable_r_v<T, Factory>)
                        {
                            data[hash] = (*factory_holder)();
                        }
                        else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                        {
                            data[hash] = co_await (*factory_holder)();
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
                if (sub_container::needs_factory_insert(data, hash))
                {
                    if constexpr (std::is_invocable_r_v<T, Factory>)
                    {
                        data[hash] = (*factory_holder)();
                    }
                    else if constexpr (std::is_invocable_r_v<async::task<T>, Factory>)
                    {
                        data[hash] = co_await (*factory_holder)();
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

    typename sub_container_type::read_guard enter_read(HashType hash) const
    {
        return this->bucket(hash)->enter_read(hash);
    }

    std::optional<typename sub_container_type::read_guard> try_enter_read(HashType hash) const
    {
        return this->bucket(hash)->try_enter_read(hash);
    }

    typename sub_container_type::write_guard enter_write(HashType hash)
    {
        return this->bucket(hash)->enter_write(hash);
    }

    async::task<typename sub_container_type::async_read_guard> enter_read_async(HashType hash) const
    {
        co_return co_await this->bucket(hash)->enter_read_async(hash);
    }

    async::task<typename sub_container_type::async_write_guard> enter_write_async(HashType hash)
    {
        co_return co_await this->bucket(hash)->enter_write_async(hash);
    }

    async::task<std::optional<typename sub_container_type::async_read_guard>> try_enter_read_async(HashType hash) const
    {
        co_return co_await this->bucket(hash)->try_enter_read_async(hash);
    }

    async::task<std::optional<typename sub_container_type::async_write_guard>> try_enter_write_async(HashType hash)
    {
        co_return co_await this->bucket(hash)->try_enter_write_async(hash);
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
