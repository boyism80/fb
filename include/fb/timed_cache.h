#ifndef __FB_TIMED_CACHE_H__
#define __FB_TIMED_CACHE_H__

#include <async/task.h>
#include <async/task_completion_source.h>
#include <fb/async_executor.h>

#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fb {

template <typename Key, typename Value>
class timed_cache
{
public:
    using clock     = std::chrono::steady_clock;
    using value_ptr = std::shared_ptr<const Value>;
    using fetcher   = std::function<async::task<Value>()>;

private:
    using fetch_waiter = std::shared_ptr<async::task_completion_source<void>>;

    struct entry
    {
        value_ptr                 value;
        clock::time_point         expires_at = {};
        bool                      fetching   = false;
        std::vector<fetch_waiter> waiters;
    };

private:
    fb::async_executor&            _executor;
    const clock::duration          _ttl;
    std::mutex                     _mutex;
    std::unordered_map<Key, entry> _entries;

public:
    timed_cache(fb::async_executor& executor, clock::duration ttl) :
        _executor(executor),
        _ttl(ttl)
    { }

    timed_cache(const timed_cache&)             = delete;
    timed_cache& operator= (const timed_cache&) = delete;

public:
    async::task<value_ptr> get(const Key& key, fetcher fetch)
    {
        auto thread = this->_executor.threads.current();
        auto waiter = fetch_waiter{};
        {
            auto lock = std::lock_guard(this->_mutex);
            auto now  = clock::now();
            for (auto it = this->_entries.begin(); it != this->_entries.end();)
            {
                if (it->first != key && it->second.fetching == false && it->second.expires_at <= now)
                    it = this->_entries.erase(it);
                else
                    ++it;
            }

            auto& found = this->_entries[key];
            if (found.value != nullptr && now < found.expires_at)
            {
                co_return found.value;
            }
            else if (found.fetching)
            {
                waiter = std::make_shared<async::task_completion_source<void>>();
                found.waiters.push_back(waiter);
            }
            else
            {
                found.fetching = true;
            }
        }

        if (waiter != nullptr)
        {
            co_await waiter->task();
            if (thread != nullptr)
                co_await thread->switching();

            auto lock  = std::lock_guard(this->_mutex);
            auto found = this->_entries.find(key);
            if (found == this->_entries.end())
                co_return nullptr;
            co_return found->second.value;
        }

        auto loaded = value_ptr{};
        auto error  = std::exception_ptr{};
        try
        {
            loaded = std::make_shared<const Value>(co_await fetch());
        }
        catch (...)
        {
            error = std::current_exception();
        }

        auto waiters = std::vector<fetch_waiter>{};
        {
            auto  lock     = std::lock_guard(this->_mutex);
            auto& found    = this->_entries[key];
            found.fetching = false;
            if (loaded != nullptr)
            {
                found.value      = loaded;
                found.expires_at = clock::now() + this->_ttl;
            }
            waiters = std::move(found.waiters);
        }

        for (auto& waiting : waiters)
        {
            auto ignored = std::exception_ptr{};
            std::ignore  = waiting->try_set_value(ignored);
        }

        if (thread != nullptr)
            co_await thread->switching();

        if (error != nullptr)
            std::rethrow_exception(error);

        co_return loaded;
    }
};

} // namespace fb

#endif // !__FB_TIMED_CACHE_H__
