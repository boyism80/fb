#include <fb/game/lazy_container.h>

#include <fb/game/castle.h>
#include <fb/game/clan.h>
#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/logger.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace fb::game {

template <typename Entity, size_t NumBuckets, typename Key>
lazy_container<Entity, NumBuckets, Key>::guard::guard() :
    _empty(nullptr)
{ }

template <typename Entity, size_t NumBuckets, typename Key>
lazy_container<Entity, NumBuckets, Key>::guard::guard(write_guard&& guard) :
    _write_guard(std::move(guard)),
    _empty(nullptr)
{ }

template <typename Entity, size_t NumBuckets, typename Key>
typename lazy_container<Entity, NumBuckets, Key>::entity_ptr&
lazy_container<Entity, NumBuckets, Key>::guard::value() noexcept
{
    if (this->_write_guard.has_value())
        return this->_write_guard->value();
    return this->_empty;
}

template <typename Entity, size_t NumBuckets, typename Key>
const typename lazy_container<Entity, NumBuckets, Key>::entity_ptr&
lazy_container<Entity, NumBuckets, Key>::guard::value() const noexcept
{
    if (this->_write_guard.has_value())
        return this->_write_guard->value();
    return this->_empty;
}

template <typename Entity, size_t NumBuckets, typename Key>
bool lazy_container<Entity, NumBuckets, Key>::guard::holds_lock() const noexcept
{
    return this->_write_guard.has_value();
}

template <typename Entity, size_t NumBuckets, typename Key>
lazy_container<Entity, NumBuckets, Key>::lazy_container(server& server) :
    super([](const entity_ptr& entity) -> Key {
        return entity->id();
    }),
    _server(server)
{ }

template <typename Entity, size_t NumBuckets, typename Key>
typename lazy_container<Entity, NumBuckets, Key>::fetch_waiter
lazy_container<Entity, NumBuckets, Key>::join_fetch(Key id)
{
    auto _     = std::lock_guard(this->_fetch_lock);
    auto found = this->_fetching.find(id);
    if (found == this->_fetching.end())
    {
        this->_fetching.emplace(id, std::vector<fetch_waiter>{});
        return nullptr;
    }

    auto waiter = std::make_shared<async::task_completion_source<void>>();
    found->second.push_back(waiter);
    return waiter;
}

template <typename Entity, size_t NumBuckets, typename Key>
void lazy_container<Entity, NumBuckets, Key>::end_fetch(Key id)
{
    auto waiters = std::vector<fetch_waiter>{};
    {
        auto _     = std::lock_guard(this->_fetch_lock);
        auto found = this->_fetching.find(id);
        if (found == this->_fetching.end())
            return;

        waiters = std::move(found->second);
        this->_fetching.erase(found);
    }

    for (auto& waiter : waiters)
    {
        auto error = std::exception_ptr{};
        if (waiter->try_set_value(error) == false && error != nullptr)
            fb::logger::warn("lazy_container fetch waiter failed (id: {})", id);
    }
}

template <typename Entity, size_t NumBuckets, typename Key>
async::task<typename lazy_container<Entity, NumBuckets, Key>::guard>
lazy_container<Entity, NumBuckets, Key>::enter_write_impl(Key id, fb::thread& thread_ref)
{
    {
        auto looked = co_await this->lookup_write_wait(id);
        if (looked.guard.has_value())
        {
            if (looked.guard->value() != nullptr)
                co_return guard(std::move(*looked.guard));

            looked.guard.reset();
            this->erase(id);
        }
    }

    // A second fetch for the same id waits for the first one, then reads what it stored.
    auto waiter = this->join_fetch(id);
    if (waiter != nullptr)
    {
        co_await waiter->task();
        co_await thread_ref.switching();

        auto looked = co_await this->lookup_write_wait(id);
        if (looked.guard.has_value() && looked.guard->value() != nullptr)
            co_return guard(std::move(*looked.guard));
        co_return guard{};
    }

    entity_ptr loaded;
    auto       failed = false;
    try
    {
        loaded = co_await this->fetch(id);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("lazy_container fetch failed (id: {}): {}", id, e.what());
        failed = true;
    }
    catch (...)
    {
        this->end_fetch(id);
        throw;
    }

    co_await thread_ref.switching();

    // A missing row is not stored. The next lookup fetches again.
    if (failed || loaded == nullptr)
    {
        this->end_fetch(id);
        co_return guard{};
    }

    auto held = co_await this->merge_write_wait(id, [loaded](const std::optional<entity_ptr>& current) -> entity_ptr {
        if (current.has_value() && current.value() != nullptr)
            return current.value();
        return loaded;
    });
    this->end_fetch(id);
    co_return guard(std::move(held));
}

template <typename Entity, size_t NumBuckets, typename Key>
async::task<typename lazy_container<Entity, NumBuckets, Key>::guard>
lazy_container<Entity, NumBuckets, Key>::enter_write(Key id)
{
    auto* thread = this->_server.threads.current();
    if (thread == nullptr)
        throw std::runtime_error(std::format("No thread available for enter_write (id: {})", id));

    co_return co_await this->enter_write_impl(id, *thread);
}

template <typename Entity, size_t NumBuckets, typename Key>
bool lazy_container<Entity, NumBuckets, Key>::contains(hash_type id) const
{
    return this->try_enter_read(id).has_value();
}

template class lazy_container<clan>::guard;
template class lazy_container<group, 16, uint64_t>::guard;
template class lazy_container<castle>::guard;

template lazy_container<clan>::lazy_container(server&);
template lazy_container<group, 16, uint64_t>::lazy_container(server&);
template lazy_container<castle>::lazy_container(server&);

template async::task<lazy_container<clan>::guard> lazy_container<clan>::enter_write_impl(uint32_t, fb::thread&);
template async::task<lazy_container<group, 16, uint64_t>::guard>
lazy_container<group, 16, uint64_t>::enter_write_impl(uint64_t, fb::thread&);
template async::task<lazy_container<castle>::guard> lazy_container<castle>::enter_write_impl(uint32_t, fb::thread&);

template async::task<lazy_container<clan>::guard> lazy_container<clan>::enter_write(uint32_t);
template async::task<lazy_container<group, 16, uint64_t>::guard>
                                                    lazy_container<group, 16, uint64_t>::enter_write(uint64_t);
template async::task<lazy_container<castle>::guard> lazy_container<castle>::enter_write(uint32_t);

template bool lazy_container<clan>::contains(hash_type) const;
template bool lazy_container<group, 16, uint64_t>::contains(hash_type) const;
template bool lazy_container<castle>::contains(hash_type) const;

} // namespace fb::game
