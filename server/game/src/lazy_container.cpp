#include <fb/game/lazy_container.h>

#include <fb/game/castle.h>
#include <fb/game/clan.h>
#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/logger.h>

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <exception>
#include <format>
#include <stdexcept>
#include <utility>

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
bool lazy_container<Entity, NumBuckets, Key>::begin_fetch(Key id)
{
    auto _ = std::lock_guard(this->_fetch_lock);
    return this->_fetching.insert(id).second;
}

template <typename Entity, size_t NumBuckets, typename Key>
void lazy_container<Entity, NumBuckets, Key>::end_fetch(Key id)
{
    auto _ = std::lock_guard(this->_fetch_lock);
    this->_fetching.erase(id);
}

template <typename Entity, size_t NumBuckets, typename Key>
async::task<typename lazy_container<Entity, NumBuckets, Key>::guard>
lazy_container<Entity, NumBuckets, Key>::enter_write_impl(Key id, fb::thread& thread_ref)
{
    // A busy bucket lock is not a cache miss. retry_exception yields this coroutine before the next attempt.
    // A second fetch for the same id waits until the first one finishes, then reads that result.
    while (true)
    {
        auto missing = false;
        for (auto attempt = 0; attempt < 4; ++attempt)
        {
            auto looked = co_await this->lookup_write_async(id);
            if (looked.guard.has_value() && looked.guard->value() == nullptr)
            {
                looked.guard.reset();
                this->erase(id);
                continue;
            }
            if (looked.guard.has_value())
                co_return guard(std::move(*looked.guard));
            if (looked.missing)
            {
                missing = true;
                break;
            }
        }
        if (missing == false)
            throw fb::thread::retry_exception{};
        if (this->begin_fetch(id))
            break;

        co_await thread_ref.sleep(fb::model::timespan(std::chrono::milliseconds(10)));
    }

    entity_ptr loaded;
    auto       failed = false;
    try
    {
        loaded = co_await this->fetch(id);
    }
    catch (const std::exception& e)
    {
        this->end_fetch(id);
        fb::logger::warn("lazy_container fetch failed (id: {}): {}", id, e.what());
        failed = true;
    }
    catch (...)
    {
        this->end_fetch(id);
        throw;
    }

    co_await thread_ref.switching();
    if (failed)
        co_return guard{};

    // A missing row is not stored. The next lookup fetches again.
    if (loaded == nullptr)
    {
        this->end_fetch(id);
        co_return guard{};
    }

    {
        const auto inserted =
            co_await this->try_merge_slot(id, [loaded](const std::optional<entity_ptr>&) -> entity_ptr {
                return loaded;
            });
        if (inserted == false)
        {
            this->end_fetch(id);
            throw fb::thread::retry_exception{};
        }
    }

    this->end_fetch(id);

    // The bucket lock is released after the insert. An erase in that gap removes the key, and taking the
    // guard must retry instead of treating the key as permanently missing.
    for (auto attempt = 0; attempt < 4; ++attempt)
    {
        auto looked = co_await this->lookup_write_async(id);
        if (looked.guard.has_value())
            co_return guard(std::move(*looked.guard));
        if (looked.missing)
            throw fb::thread::retry_exception{};
    }
    throw fb::thread::retry_exception{};
}

template <typename Entity, size_t NumBuckets, typename Key>
async::task<typename lazy_container<Entity, NumBuckets, Key>::guard>
lazy_container<Entity, NumBuckets, Key>::enter_write(Key id)
{
    auto* thread = this->_server.threads.current();
    if (thread == nullptr)
        throw std::runtime_error(std::format("No thread available for enter_write (id: {})", id));

    auto builder        = thread->template new_builder<guard>();
    builder.retry_count = 10;
    builder.func        = [this, id](auto& thread_ref) -> async::task<guard> {
        co_return co_await this->enter_write_impl(id, thread_ref);
    };
    co_return co_await builder.dispatch();
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
