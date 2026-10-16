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
#include <stdexcept>
#include <utility>

namespace fb::game {

template <typename Entity, size_t NumBuckets>
lazy_container<Entity, NumBuckets>::guard::guard() :
    _empty(nullptr)
{ }

template <typename Entity, size_t NumBuckets>
lazy_container<Entity, NumBuckets>::guard::guard(write_guard&& guard) :
    _write_guard(std::move(guard)),
    _empty(nullptr)
{ }

template <typename Entity, size_t NumBuckets>
typename lazy_container<Entity, NumBuckets>::entity_ptr& lazy_container<Entity, NumBuckets>::guard::value() noexcept
{
    if (this->_write_guard.has_value())
        return this->_write_guard->value();
    return this->_empty;
}

template <typename Entity, size_t NumBuckets>
const typename lazy_container<Entity, NumBuckets>::entity_ptr&
lazy_container<Entity, NumBuckets>::guard::value() const noexcept
{
    if (this->_write_guard.has_value())
        return this->_write_guard->value();
    return this->_empty;
}

template <typename Entity, size_t NumBuckets>
bool lazy_container<Entity, NumBuckets>::guard::holds_lock() const noexcept
{
    return this->_write_guard.has_value();
}

template <typename Entity, size_t NumBuckets>
lazy_container<Entity, NumBuckets>::lazy_container(server& server) :
    super([](const entity_ptr& entity) -> uint32_t {
        return entity->id();
    }),
    _server(server)
{ }

template <typename Entity, size_t NumBuckets>
async::task<typename lazy_container<Entity, NumBuckets>::guard>
lazy_container<Entity, NumBuckets>::enter_write_impl(uint32_t id, fb::thread& thread_ref)
{
    // A busy bucket lock is not a cache miss. retry_exception yields this coroutine before the next attempt.
    {
        auto missing = false;
        for (auto attempt = 0; attempt < 4; ++attempt)
        {
            auto looked = co_await this->lookup_write_async(id);
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
    }

    fetched loaded{nullptr, fb::model::datetime("1970-01-01 00:00:00")};
    try
    {
        loaded = co_await this->fetch(id);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("lazy_container fetch failed (id: {}): {}", id, e.what());
        co_return guard{};
    }

    co_await thread_ref.switching();

    // A nullptr slot is a tombstone, not an empty key. Assign through merge so an older fetch cannot replace it.
    {
        const auto inserted =
            co_await this->try_merge_slot(id,
                                          [this, id, loaded](const std::optional<entity_ptr>& current) -> entity_ptr {
                                              return this->merge_fetched(id, current, loaded);
                                          });
        if (inserted == false)
            throw fb::thread::retry_exception{};
    }

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

template <typename Entity, size_t NumBuckets>
async::task<typename lazy_container<Entity, NumBuckets>::guard>
lazy_container<Entity, NumBuckets>::open(uint32_t id, const fb::model::datetime& at)
{
    {
        auto held = co_await this->enter_write(id);
        if (held.holds_lock() == false)
            co_return guard{};
        if (held.value() != nullptr)
            co_return std::move(held);
        if (this->accept(id, at) == false)
            co_return guard{};
    }

    this->erase(id);
    co_return co_await this->enter_write(id);
}

template <typename Entity, size_t NumBuckets>
async::task<typename lazy_container<Entity, NumBuckets>::guard>
lazy_container<Entity, NumBuckets>::enter_write(uint32_t id)
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

template <typename Entity, size_t NumBuckets>
bool lazy_container<Entity, NumBuckets>::contains(hash_type id) const
{
    return this->try_enter_read(id).has_value();
}

template class lazy_container<clan>::guard;
template class lazy_container<group>::guard;
template class lazy_container<castle>::guard;

template lazy_container<clan>::lazy_container(server&);
template lazy_container<group>::lazy_container(server&);
template lazy_container<castle>::lazy_container(server&);

template async::task<lazy_container<clan>::guard>   lazy_container<clan>::enter_write_impl(uint32_t, fb::thread&);
template async::task<lazy_container<group>::guard>  lazy_container<group>::enter_write_impl(uint32_t, fb::thread&);
template async::task<lazy_container<castle>::guard> lazy_container<castle>::enter_write_impl(uint32_t, fb::thread&);

template async::task<lazy_container<clan>::guard>   lazy_container<clan>::enter_write(uint32_t);
template async::task<lazy_container<group>::guard>  lazy_container<group>::enter_write(uint32_t);
template async::task<lazy_container<castle>::guard> lazy_container<castle>::enter_write(uint32_t);

template async::task<lazy_container<clan>::guard>   lazy_container<clan>::open(uint32_t, const fb::model::datetime&);
template async::task<lazy_container<group>::guard>  lazy_container<group>::open(uint32_t, const fb::model::datetime&);
template async::task<lazy_container<castle>::guard> lazy_container<castle>::open(uint32_t, const fb::model::datetime&);

template bool lazy_container<clan>::contains(hash_type) const;
template bool lazy_container<group>::contains(hash_type) const;
template bool lazy_container<castle>::contains(hash_type) const;

} // namespace fb::game
