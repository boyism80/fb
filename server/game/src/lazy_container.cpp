#include <fb/game/clan.h>
#include <fb/game/group.h>
#include <fb/game/lazy_container.h>
#include <fb/game/server.h>
#include <fb/logger.h>

#include <format>
#include <stdexcept>

namespace fb::game {

template <typename Entity, size_t NumBuckets>
lazy_container<Entity, NumBuckets>::ensure_guard::ensure_guard() :
    _empty(nullptr)
{ }

template <typename Entity, size_t NumBuckets>
lazy_container<Entity, NumBuckets>::ensure_guard::ensure_guard(write_guard&& guard) :
    _write_guard(std::move(guard)),
    _empty(nullptr)
{ }

template <typename Entity, size_t NumBuckets>
typename lazy_container<Entity, NumBuckets>::entity_ptr&
lazy_container<Entity, NumBuckets>::ensure_guard::value() noexcept
{
    if (this->_write_guard.has_value())
        return this->_write_guard->value();
    return this->_empty;
}

template <typename Entity, size_t NumBuckets>
const typename lazy_container<Entity, NumBuckets>::entity_ptr&
lazy_container<Entity, NumBuckets>::ensure_guard::value() const noexcept
{
    if (this->_write_guard.has_value())
        return this->_write_guard->value();
    return this->_empty;
}

template <typename Entity, size_t NumBuckets>
bool lazy_container<Entity, NumBuckets>::ensure_guard::holds_lock() const noexcept
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
async::task<typename lazy_container<Entity, NumBuckets>::ensure_guard>
lazy_container<Entity, NumBuckets>::ensure_impl(uint32_t id, fb::thread& thread_ref)
{
    // Phase 1 — cache lookup (non-blocking write lock attempt; released when hit guard is moved or destroyed)
    {
        auto hit = co_await this->try_enter_write_async(id);
        if (hit.has_value())
            co_return ensure_guard(std::move(*hit));
    }

    // Phase 2 — resolve outside container lock (HTTP fetch, thread switch)
    entity_ptr entity;
    try
    {
        entity = co_await this->fetch(id);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("lazy_container fetch failed (id: {}): {}", id, e.what());
        co_return ensure_guard{};
    }

    co_await thread_ref.switching();

    // Phase 3 — insert resolved value, including nullptr tombstone (write lock released when try_async_write
    // completes)
    {
        const auto inserted = co_await this->try_async_write(
            id,
            [](auto&) -> async::task<void> {
                co_return;
            },
            [entity]() -> entity_ptr {
                return entity;
            });
        if (inserted == false)
            throw fb::thread::retry_exception{};
    }

    // Phase 4 — return write guard to caller (lock held until ensure_guard is destroyed)
    co_return ensure_guard(co_await this->enter_write_async(id));
}

template <typename Entity, size_t NumBuckets>
async::task<typename lazy_container<Entity, NumBuckets>::ensure_guard>
lazy_container<Entity, NumBuckets>::ensure(uint32_t id)
{
    auto* thread = this->_server.threads.current();
    if (thread == nullptr)
        throw std::runtime_error(std::format("No thread available for ensure (id: {})", id));

    auto builder        = thread->template new_builder<ensure_guard>();
    builder.retry_count = 10;
    builder.func        = [this, id](auto& thread_ref) -> async::task<ensure_guard> {
        co_return co_await this->ensure_impl(id, thread_ref);
    };
    co_return co_await builder.dispatch();
}

template <typename Entity, size_t NumBuckets>
bool lazy_container<Entity, NumBuckets>::contains(hash_type id) const
{
    return this->try_enter_read(id).has_value();
}

template class lazy_container<clan>::ensure_guard;
template class lazy_container<group>::ensure_guard;

template lazy_container<clan>::lazy_container(server&);
template lazy_container<group>::lazy_container(server&);

template async::task<lazy_container<clan>::ensure_guard>  lazy_container<clan>::ensure_impl(uint32_t, fb::thread&);
template async::task<lazy_container<group>::ensure_guard> lazy_container<group>::ensure_impl(uint32_t, fb::thread&);

template async::task<lazy_container<clan>::ensure_guard>  lazy_container<clan>::ensure(uint32_t);
template async::task<lazy_container<group>::ensure_guard> lazy_container<group>::ensure(uint32_t);

template bool lazy_container<clan>::contains(hash_type) const;
template bool lazy_container<group>::contains(hash_type) const;

} // namespace fb::game
