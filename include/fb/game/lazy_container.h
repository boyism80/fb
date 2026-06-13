#ifndef __FB_GAME_LAZY_CONTAINER_H__
#define __FB_GAME_LAZY_CONTAINER_H__

#include <fb/shard_container.h>
#include <fb/thread.h>
#include <async/task.h>
#include <cstdint>
#include <memory>
#include <optional>

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

        entity_ptr&       value() noexcept;
        const entity_ptr& value() const noexcept;
        bool              holds_lock() const noexcept;

    private:
        friend class lazy_container;

        std::optional<write_guard> _write_guard;
        entity_ptr                 _empty;

        ensure_guard();
        explicit ensure_guard(write_guard&& guard);
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

    async::task<ensure_guard> ensure_impl(uint32_t id, fb::thread& thread_ref);

public:
    explicit lazy_container(server& server);

    lazy_container(const lazy_container&) = delete;
    lazy_container(lazy_container&&)      = delete;
    virtual ~lazy_container()             = default;

    async::task<ensure_guard> ensure(uint32_t id);
    bool                      contains(hash_type id) const;
};

} // namespace fb::game

#endif
