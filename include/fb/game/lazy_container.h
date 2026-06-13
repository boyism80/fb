#ifndef __FB_GAME_LAZY_CONTAINER_H__
#define __FB_GAME_LAZY_CONTAINER_H__

#include <fb/shard_container.h>
#include <fb/thread.h>
#include <fb/logger.h>
#include <async/task.h>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <cstdint>

namespace fb::game {

class server;

template <typename Entity, size_t NumBuckets = 16>
class lazy_container : protected fb::sharded_container<std::shared_ptr<Entity>, NumBuckets>
{
public:
    using entity_ptr  = std::shared_ptr<Entity>;
    using hash_type   = uint32_t;
    using super       = fb::sharded_container<entity_ptr, NumBuckets>;
    using write_guard = typename super::sub_container_type::async_write_guard;
    using read_guard  = typename super::sub_container_type::read_guard;

    class ensure_guard
    {
    public:
        ensure_guard(ensure_guard&&) noexcept             = default;
        ensure_guard& operator= (ensure_guard&&) noexcept = delete;
        ensure_guard(const ensure_guard&)                 = delete;
        ensure_guard& operator= (const ensure_guard&)     = delete;

        entity_ptr& value() noexcept
        {
            if (this->_write_guard.has_value())
                return this->_write_guard->value();
            return this->_empty;
        }

        const entity_ptr& value() const noexcept
        {
            if (this->_write_guard.has_value())
                return this->_write_guard->value();
            return this->_empty;
        }

        bool holds_lock() const noexcept
        {
            return this->_write_guard.has_value();
        }

    private:
        friend class lazy_container;

        std::optional<write_guard> _write_guard;
        entity_ptr                 _empty;

        ensure_guard() :
            _empty(nullptr)
        { }

        explicit ensure_guard(write_guard&& guard) :
            _write_guard(std::move(guard)),
            _empty(nullptr)
        { }
    };

public:
    using super::async_write;
    using super::enter_read;
    using super::erase;
    using super::insert;
    using super::keys;
    using super::try_enter_read;
    using super::write;

protected:
    server& _server;

protected:
    virtual async::task<entity_ptr> fetch(uint32_t id) = 0;

    async::task<ensure_guard> ensure_impl(uint32_t id, fb::thread& thread_ref)
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

public:
    explicit lazy_container(server& server) :
        super([](const entity_ptr& entity) -> uint32_t {
            return entity->id();
        }),
        _server(server)
    { }

    lazy_container(const lazy_container&) = delete;
    lazy_container(lazy_container&&)      = delete;
    virtual ~lazy_container()             = default;

    async::task<ensure_guard> ensure(uint32_t id)
    {
        auto* thread = this->_server.threads.current();
        if (thread == nullptr)
            throw std::runtime_error(std::format("No thread available for ensure (id: {})", id));

        auto builder        = thread->new_builder<ensure_guard>();
        builder.retry_count = 10;
        builder.func        = [this, id](auto& thread_ref) -> async::task<ensure_guard> {
            co_return co_await this->ensure_impl(id, thread_ref);
        };
        co_return co_await builder.dispatch();
    }

    bool contains(hash_type id) const
    {
        return this->try_enter_read(id).has_value();
    }
};

} // namespace fb::game

#endif
