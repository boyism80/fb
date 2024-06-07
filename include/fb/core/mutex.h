#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <map>
#include <mutex>
#include <fb/core/abstract.h>
#include <fb/core/coroutine.h>
#include <fb/core/concurrent.h>

namespace fb {

class mutex : fb::concurrent
{
private:
    using mutex_pool = std::map<std::string, std::unique_ptr<std::mutex>>;

private:
    icontext&                           _owner;
    mutex_pool                          _pool;
    std::mutex                          _mutex;

public:
    mutex(icontext& owner) : _owner(owner)
    {}
    ~mutex() = default;

private:
    template <typename T>
    fb::task<void> handle_locked(fb::awaiter<T>& awaiter, const std::function<fb::task<T>(std::shared_ptr<fb::mst>&)>& fn, std::shared_ptr<fb::mst> current, const std::string key, std::mutex& mutex)
    {
        mutex.lock();
        try
        {
            auto result = co_await fn(current);
            awaiter.result = &result;

            concurrent::add(current);
        }
        catch(std::exception& e)
        {
            awaiter.error = std::make_unique<std::runtime_error>(e.what());
        }
        mutex.unlock();
        awaiter.handler.resume();
    }

    template <typename T>
    bool try_lock(const std::string& key, fb::awaiter<T>& awaiter, const std::function<fb::task<T>(std::shared_ptr<fb::mst>&)>& fn, fb::thread* thread, std::shared_ptr<fb::mst>& trans)
    {
        std::mutex* mutex = nullptr;
        {
            auto _ = std::lock_guard<std::mutex>(this->_mutex);
            if (this->_pool.contains(key) == false)
                this->_pool.insert({key, std::make_unique<std::mutex>()});

            mutex = this->_pool[key].get();
        }

        auto current = concurrent::alloc(key, trans);
        try
        {
            concurrent::assert_dead_lock(current);
        }
        catch(std::exception& e)
        {
            awaiter.error = std::make_unique<std::runtime_error>(e.what());
            awaiter.handler.resume();
            return false;
        }

        if (thread != nullptr)
        {
            thread->dispatch([this, &awaiter, &fn, current, key, mutex](uint8_t) mutable 
            {
                this->handle_locked(awaiter, fn, current, key, *mutex);
            });
        }
        else
        {
            this->handle_locked(awaiter, fn, current, key, *mutex);
        }

        return true;
    }

public:
    template <typename T>
    fb::awaiter<T> sync(const std::string& key, const std::function<fb::task<T>(std::shared_ptr<fb::mst>&)>& fn, std::shared_ptr<fb::mst>& trans)
    {
        return fb::awaiter<T>([this, key, &fn, trans](auto& awaiter) mutable
        {
            auto thread = this->_owner.current_thread();
            this->try_lock(key, awaiter, fn, thread, trans);
        });
    }

    template <typename T>
    fb::awaiter<T> sync(const std::string& key, const std::function<fb::task<T>(std::shared_ptr<fb::mst>&)>& fn)
    {
        auto empty_ptr = std::shared_ptr<fb::mst>(nullptr);
        return this->sync(key, fn, empty_ptr);
    }
};

}

#endif