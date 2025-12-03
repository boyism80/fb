#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <fb/async_executor.h>
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
    fb::async_executor& _executor;
    mutex_pool          _pool;
    std::mutex          _mutex;

public:
    mutex(fb::async_executor& executor) :
        _executor(executor)
    { }

    ~mutex() = default;

private:
    template <typename T>
    [[nodiscard]] async::task<T>
    on_locked(std::shared_ptr<async::task_completion_source<T>> promise, const async_wait_func<T>& fn, fb::dead_lock_detector& current, const std::string key, std::mutex& mutex)
    {
        {
            auto _ = std::lock_guard(mutex);
            try
            {
                if constexpr (std::is_same_v<T, void>)
                {
                    co_await fn(current);
                    promise->set_value();
                }
                else
                {
                    promise->set_value(co_await fn(current));
                }

                concurrent::add(current);
            }
            catch (std::exception& e)
            {
                promise->set_exception(std::make_exception_ptr(e));
            }
        }
    }

    template <typename T>
    T on_locked(const sync_wait_func<T>& fn, fb::dead_lock_detector& current, const std::string key, std::mutex& mutex)
    {
        auto _ = std::lock_guard(mutex);

        auto result = fn(current);
        concurrent::add(current);
        return result;
    }

    template <typename T>
    T on_locked(const sync_peek_func<T>& fn, const std::string key, std::mutex& mutex)
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
    async::task<bool>
    lock(const std::string& key, std::shared_ptr<async::task_completion_source<T>> promise, const async_wait_func<T>& fn, fb::thread* thread, fb::dead_lock_detector& trans)
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
            co_return false;
        }

        if (thread != nullptr)
        {
            thread->dispatch([this, promise, &fn, &current, key, mutex](auto& thread) mutable -> async::task<void> {
                co_await this->on_locked(promise, fn, current, key, *mutex);
            });
        }
        else
        {
            co_await this->on_locked(promise, fn, current, key, *mutex);
        }

        co_return true;
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
                this->on_locked(promise, fn, key, *mutex);
            });
        }
        else
        {
            this->on_locked(promise, fn, key, *mutex);
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

        return this->on_locked(fn, current, key, *mutex);
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

        return this->on_locked(fn, key, *mutex);
    }

public:
    template <typename T>
    [[nodiscard]] async::task<T> sync(const std::string& key, const async_wait_func<T>& fn, fb::dead_lock_detector& trans)
    {
        auto thread  = this->_executor.threads.current();
        auto promise = std::make_shared<async::task_completion_source<T>>();

        this->lock(key, promise, fn, thread, trans);
        return promise->task();
    }

    template <typename T>
    [[nodiscard]] async::task<T> sync(const std::string& key, const async_wait_func<T>& fn)
    {
        if constexpr (std::is_same_v<T, void>)
        {
            co_await this->sync(key, fn, this->root);
        }
        else
        {
            co_return co_await this->sync(key, fn, this->root);
        }
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
            auto thread = this->_executor.threads.current();
            this->try_lock(key, promise, fn, thread);
        });
    }
};

} // namespace fb

#endif // !__MUTEX_H__