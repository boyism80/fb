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
#include <unordered_map>

namespace fb::game {

class server;

inline fb::model::datetime version_time(std::string_view text)
{
    if (text.empty())
        return fb::model::datetime("1970-01-01 00:00:00");
    return fb::model::datetime(text);
}

template <typename Entity, size_t NumBuckets = 16>
class lazy_container : protected fb::sharded_container<std::shared_ptr<Entity>, NumBuckets>
{
public:
    using entity_ptr  = std::shared_ptr<Entity>;
    using hash_type   = uint32_t;
    using super       = fb::sharded_container<entity_ptr, NumBuckets>;
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

    struct fetched
    {
        entity_ptr          entity;
        fb::model::datetime at;
    };

protected:
    server&                                           _server;
    mutable std::mutex                                _as_of_lock;
    std::unordered_map<uint32_t, fb::model::datetime> _as_of;

protected:
    virtual async::task<fetched> fetch(uint32_t id) = 0;

    bool recorded(uint32_t id, const fb::model::datetime& at) const
    {
        auto _  = std::lock_guard(this->_as_of_lock);
        auto it = this->_as_of.find(id);
        return it != this->_as_of.end() && it->second >= at;
    }

    bool accept(uint32_t id, const fb::model::datetime& at) const
    {
        return this->recorded(id, at) == false;
    }

    void note(uint32_t id, const fb::model::datetime& at)
    {
        auto _  = std::lock_guard(this->_as_of_lock);
        auto it = this->_as_of.find(id);
        if (it == this->_as_of.end())
            this->_as_of.emplace(id, at);
        else if (at > it->second)
            it->second = at;
    }

    virtual entity_ptr merge_fetched(uint32_t id, const std::optional<entity_ptr>& current, const fetched& incoming)
    {
        static_cast<void>(id);
        if (current.has_value() && *current != nullptr)
            return *current;
        return incoming.entity;
    }

    async::task<guard> enter_write_impl(uint32_t id, fb::thread& thread_ref);
    async::task<guard> open(uint32_t id, const fb::model::datetime& at);

public:
    explicit lazy_container(server& server);

    lazy_container(const lazy_container&) = delete;
    lazy_container(lazy_container&&)      = delete;
    virtual ~lazy_container()             = default;

    async::task<guard> enter_write(uint32_t id);
    bool               contains(hash_type id) const;
};

} // namespace fb::game

#endif
