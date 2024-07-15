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
    template <typename T>
    using async_wait_func = std::function<fb::task<T>(fb::dead_lock_detector&)>;
    template <typename T>
    using async_peek_func = std::function<fb::task<T>(void)>;
    template <typename T>
    using sync_wait_func  = std::function<T(fb::dead_lock_detector&)>;
    template <typename T>
    using sync_peek_func  = std::function<T(void)>;

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
    fb::task<void> handle_locked(fb::awaiter<T>& awaiter, const async_wait_func<T>& fn, fb::dead_lock_detector& current, const std::string key, std::mutex& mutex)
    {
        {
            auto _ = std::lock_guard(mutex);
            try
            {
                auto& result = co_await fn(current);
                awaiter.result(std::ref(result));

                concurrent::add(current);
            }
            catch(std::exception& e)
            {
                awaiter.error(e);
            }
        }
        awaiter.resume();
    }

    template <typename T>
    fb::task<void> handle_locked(fb::awaiter<T>& awaiter, const async_peek_func<T>& fn, const std::string key, std::mutex& mutex)
    {
        if(mutex.try_lock())
        {
            try
            {
                auto& result = co_await fn();
                awaiter.result(std::ref(result));
            }
            catch(std::exception& e)
            {
                awaiter.error(e);
            }

            mutex.unlock();
        }
        else
        {
            awaiter.error(fb::lock_error());
        }
        
        awaiter.resume();
    }

    template <typename T>
    T handle_locked(const sync_wait_func<T>& fn, fb::dead_lock_detector& current, const std::string key, std::mutex& mutex)
    {
        auto _ = std::lock_guard(mutex);
        
        auto result = fn(current);
        concurrent::add(current);
        return result;
    }

    template <typename T>
    T handle_locked(const sync_peek_func<T>& fn, const std::string key, std::mutex& mutex)
    {
        if(mutex.try_lock())
        {
            try
            {
                auto result = fn();
                mutex.unlock();
                return result;
            }
            catch(std::exception& e)
            {
                mutex.unlock();
                throw e;
            }
        }
        else
        {
            throw fb::lock_error();
        }
    }

    template <typename T>
    bool lock(const std::string& key, fb::awaiter<T>& awaiter, const async_wait_func<T>& fn, fb::thread* thread, fb::dead_lock_detector& trans)
    {
        std::mutex* mutex = nullptr;
        {
            auto _ = std::lock_guard(this->_mutex);
            if (this->_pool.contains(key) == false)
                this->_pool.insert({key, std::make_unique<std::mutex>()});

            mutex = this->_pool[key].get();
        }

        auto& current = static_cast<fb::mst<std::string>&>(trans).add<fb::dead_lock_detector>(key, &trans);
        try
        {
            concurrent::assert_dead_lock(current);
        }
        catch(std::exception& e)
        {
            awaiter.resume(e);
            return false;
        }

        if (thread != nullptr)
        {
            thread->dispatch([this, &awaiter, &fn, &current, key, mutex]() mutable 
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

    template <typename T>
    void try_lock(const std::string& key, fb::awaiter<T>& awaiter, const async_peek_func<T>& fn, fb::thread* thread)
    {
        std::mutex* mutex = nullptr;
        {
            auto _ = std::lock_guard(this->_mutex);
            if (this->_pool.contains(key) == false)
                this->_pool.insert({key, std::make_unique<std::mutex>()});

            mutex = this->_pool[key].get();
        }

        if (thread != nullptr)
        {
            thread->dispatch([this, &awaiter, &fn, key, mutex]() mutable 
            {
                this->handle_locked(awaiter, fn, key, *mutex);
            });
        }
        else
        {
            this->handle_locked(awaiter, fn, key, *mutex);
        }
    }

    template <typename T>
    T lock(const std::string& key, const sync_wait_func<T>& fn, fb::dead_lock_detector& trans)
    {
        std::mutex* mutex = nullptr;
        {
            auto _ = std::lock_guard(this->_mutex);
            if (this->_pool.contains(key) == false)
                this->_pool.insert({key, std::make_unique<std::mutex>()});

            mutex = this->_pool[key].get();
        }

        auto& current = static_cast<fb::mst<std::string>&>(trans).add<fb::dead_lock_detector>(key, &trans);
        concurrent::assert_dead_lock(current);

        return this->handle_locked(fn, current, key, *mutex);
    }

    template <typename T>
    T try_lock(const std::string& key, const sync_peek_func<T>& fn)
    {
        std::mutex* mutex = nullptr;
        {
            auto _ = std::lock_guard(this->_mutex);
            if (this->_pool.contains(key) == false)
                this->_pool.insert({key, std::make_unique<std::mutex>()});

            mutex = this->_pool[key].get();
        }

        return this->handle_locked(fn, key, *mutex);
    }

public:
    template <typename T>
    fb::awaiter<T> sync(const std::string& key, const async_wait_func<T>& fn, fb::dead_lock_detector& trans)
    {
        return fb::awaiter<T>([this, key, &fn, &trans](auto& awaiter) mutable
        {
            auto thread = this->_owner.current_thread();
            this->lock(key, awaiter, fn, thread, trans);
        });
    }

    template <typename T>
    fb::awaiter<T> sync(const std::string& key, const async_wait_func<T>& fn)
    {
        return this->sync(key, fn, this->root);
    }

    template <typename T>
    T sync(const std::string& key, const sync_wait_func<T>& fn, fb::dead_lock_detector& trans)
    {
        return this->lock(key, fn, trans);
    }

    template <typename T>
    T sync(const std::string& key, const sync_wait_func<T>& fn)
    {
        return this->sync(key, fn, this->root);
    }

    template <typename T>
    T try_sync(const std::string& key, const async_peek_func<T>& fn)
    {
        return fb::awaiter<T>([this, key, &fn](auto& awaiter) mutable
        {
            auto thread = this->_owner.current_thread();
            this->try_lock(key, awaiter, fn, thread);
        });
    }
};

}

#endif