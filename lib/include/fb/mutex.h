#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <map>
#include <mutex>
#include <fb/abstract.h>
#include <fb/concurrent.h>

namespace fb {

class mutex : fb::concurrent
{
private:
    using mutex_pool = std::map<std::string, std::unique_ptr<std::mutex>>;
    template <typename T>
    using async_wait_func = std::function<async::task<T>(fb::dead_lock_detector&)>;
    template <typename T>
    using async_peek_func = std::function<async::task<T>(void)>;
    template <typename T>
    using sync_wait_func = std::function<T(fb::dead_lock_detector&)>;
    template <typename T>
    using sync_peek_func = std::function<T(void)>;

private:
    icontext&  _owner;
    mutex_pool _pool;
    std::mutex _mutex;

public:
    mutex(icontext& owner) :
        _owner(owner)
    { }
    ~mutex() = default;

private:
    template <typename T>
    async::task<void> handle_locked(std::shared_ptr<async::task_completion_source<T>> promise,
                                    const async_wait_func<T>&                         fn,
                                    fb::dead_lock_detector&                           current,
                                    const std::string                                 key,
                                    std::mutex&                                       mutex)
    {
        {
            auto _ = std::lock_guard(mutex);
            try
            {
                promise->set_value(co_await fn(current));

                concurrent::add(current);
            }
            catch (std::exception& e)
            {
                promise->set_exception(std::make_exception_ptr(e));
            }
        }
    }

    template <typename T>
    async::task<void> handle_locked(std::shared_ptr<async::task_completion_source<T>> promise, const async_peek_func<T>& fn, const std::string key, std::mutex& mutex)
    {
        if (mutex.try_lock())
        {
            try
            {
                auto& result = co_await fn();
                promise->set_result(result);
            }
            catch (std::exception& e)
            {
                promise->set_error(e);
            }

            mutex.unlock();
        }
        else
        {
            promise->set_error(fb::lock_error());
        }
    }

    template <typename T>
    T handle_locked(const sync_wait_func<T>& fn, fb::dead_lock_detector& current, const std::string key, std::mutex& mutex)
    {
        auto _      = std::lock_guard(mutex);

        auto result = fn(current);
        concurrent::add(current);
        return result;
    }

    template <typename T>
    T handle_locked(const sync_peek_func<T>& fn, const std::string key, std::mutex& mutex)
    {
        if (mutex.try_lock())
        {
            try
            {
                auto result = fn();
                mutex.unlock();
                return result;
            }
            catch (std::exception& e)
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
    bool lock(const std::string& key, std::shared_ptr<async::task_completion_source<T>> promise, const async_wait_func<T>& fn, fb::thread* thread, fb::dead_lock_detector& trans)
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
        catch (std::exception& e)
        {
            promise->set_exception(std::make_exception_ptr(e));
            return false;
        }

        if (thread != nullptr)
        {
            thread->dispatch([this, promise, &fn, &current, key, mutex]() mutable -> async::task<void> {
                this->handle_locked(promise, fn, current, key, *mutex);
                co_return;
            });
        }
        else
        {
            this->handle_locked(promise, fn, current, key, *mutex);
        }

        return true;
    }

    template <typename T>
    void try_lock(const std::string& key, std::shared_ptr<async::task_completion_source<T>> promise, const async_peek_func<T>& fn, fb::thread* thread)
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
            thread->dispatch([this, promise, &fn, key, mutex]() mutable {
                this->handle_locked(promise, fn, key, *mutex);
            });
        }
        else
        {
            this->handle_locked(promise, fn, key, *mutex);
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
    async::task<T> sync(const std::string& key, const async_wait_func<T>& fn, fb::dead_lock_detector& trans)
    {
        auto thread  = this->_owner.current_thread();
        auto promise = std::make_shared<async::task_completion_source<T>>();

        this->lock(key, promise, fn, thread, trans);
        return promise->task();
    }

    template <typename T>
    async::task<T> sync(const std::string& key, const async_wait_func<T>& fn)
    {
        co_return co_await this->sync(key, fn, this->root);
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
        return async::task_completion_source<T>([this, key, &fn](auto& promise) mutable {
            auto thread = this->_owner.current_thread();
            this->try_lock(key, promise, fn, thread);
        });
    }
};

} // namespace fb

#endif