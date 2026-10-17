#ifndef __FB_GAME_LAZY_CONTAINER_H__
#define __FB_GAME_LAZY_CONTAINER_H__

#include <async/task.h>
#include <fb/model/datetime.h>
#include <fb/shard_container.h>
#include <fb/thread.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_set>

namespace fb::game {

class server;

template <typename Entity, size_t NumBuckets = 16, typename Key = uint32_t>
class lazy_container : protected fb::sharded_container<std::shared_ptr<Entity>, NumBuckets, Key>
{
public:
    using entity_ptr  = std::shared_ptr<Entity>;
    using hash_type   = Key;
    using super       = fb::sharded_container<entity_ptr, NumBuckets, Key>;
    using write_guard = typename super::sub_container_type::async_write_guard;
    using read_guard  = typename super::sub_container_type::read_guard;

    class guard
    {
    public:
        guard(guard&&) noexcept             = default;
        guard& operator= (guard&&) noexcept = delete;
        guard(const guard&)                 = delete;
        guard& operator= (const guard&)     = delete;

        entity_ptr&       value() noexcept;
        const entity_ptr& value() const noexcept;
        bool              holds_lock() const noexcept;

    private:
        friend class lazy_container;

        std::optional<write_guard> _write_guard;
        entity_ptr                 _empty;

        guard();
        explicit guard(write_guard&& guard);
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

private:
    std::mutex              _fetch_lock;
    std::unordered_set<Key> _fetching;

private:
    bool begin_fetch(Key id);
    void end_fetch(Key id);

protected:
    virtual async::task<entity_ptr> fetch(Key id) = 0;

    async::task<guard> enter_write_impl(Key id, fb::thread& thread_ref);

public:
    explicit lazy_container(server& server);

    lazy_container(const lazy_container&) = delete;
    lazy_container(lazy_container&&)      = delete;
    virtual ~lazy_container()             = default;

    async::task<guard> enter_write(Key id);
    bool               contains(hash_type id) const;
};

} // namespace fb::game

#endif
