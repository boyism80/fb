#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <fb/abstract.h>
#include <fb/concurrent.h>

namespace fb {

/**
 * @brief      Advanced mutex system with deadlock detection and named lock management.
 *
 *             This class provides a sophisticated mutex implementation that goes beyond
 *             standard mutex functionality. It maintains a pool of named mutexes,
 *             implements deadlock detection using dependency tracking, and supports
 *             both synchronous and asynchronous locking operations. The system is
 *             designed for complex multi-threaded applications where deadlock prevention
 *             and named resource locking are critical for system stability.
 */
class mutex : fb::concurrent
{
private:
    using mutex_pool = std::map<std::string, std::unique_ptr<std::mutex>>;

    /**
     * @brief      The type of the asynchronous wait function.
     */
    template <typename T>
    using async_wait_func = std::function<async::task<T>(fb::dead_lock_detector&)>;

    /**
     * @brief      The type of the asynchronous peek function.
     */
    template <typename T>
    using async_peek_func = std::function<async::task<T>(void)>;

    /**
     * @brief      The type of the synchronous wait function.
     */
    template <typename T>
    using sync_wait_func = std::function<T(fb::dead_lock_detector&)>;

    /**
     * @brief      The type of the synchronous peek function.
     */
    template <typename T>
    using sync_peek_func = std::function<T(void)>;

private:
    fb::context& _owner;
    mutex_pool   _pool;
    std::mutex   _mutex;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  owner  The owner.
     */
    mutex(fb::context& owner) :
        _owner(owner)
    { }

    /**
     * @brief      Destroys the mutex.
     */
    ~mutex() = default;

private:
    /**
     * @brief      Handles the locked state.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  promise  The promise.
     * @param[in]  fn  The function.
     * @param[in]  current  The current.
     * @param[in]  key  The key.
     * @param[in]  mutex  The mutex.
     *
     * @return     The value.
     */
    template <typename T>
    [[nodiscard]] async::task<T> handle_locked(std::shared_ptr<async::task_completion_source<T>> promise,
                                               const async_wait_func<T>&                         fn,
                                               fb::dead_lock_detector&                           current,
                                               const std::string                                 key,
                                               std::mutex&                                       mutex)
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

    /**
     * @brief      Handles the locked state.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  fn  The function.
     * @param[in]  current  The current.
     * @param[in]  key  The key.
     * @param[in]  mutex  The mutex.
     *
     * @return     The value.
     */
    template <typename T>
    T handle_locked(const sync_wait_func<T>& fn,
                    fb::dead_lock_detector&  current,
                    const std::string        key,
                    std::mutex&              mutex)
    {
        auto _ = std::lock_guard(mutex);

        auto result = fn(current);
        concurrent::add(current);
        return result;
    }

    /**
     * @brief      Handles the locked state.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  fn  The function.
     * @param[in]  current  The current.
     * @param[in]  key  The key.
     * @param[in]  mutex  The mutex.
     *
     * @return     The value.
     */
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

    /**
     * @brief      Locks the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  promise  The promise.
     * @param[in]  fn  The function.
     * @param[in]  thread  The thread.
     * @param[in]  trans  The trans.
     *
     * @return     The value.
     */
    template <typename T>
    async::task<bool> lock(const std::string&                                key,
                           std::shared_ptr<async::task_completion_source<T>> promise,
                           const async_wait_func<T>&                         fn,
                           fb::thread*                                       thread,
                           fb::dead_lock_detector&                           trans)
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
            async::awaitable_then(
                thread->dispatch([this, promise, &fn, &current, key, mutex](auto& thread) mutable -> async::task<void> {
                    co_await this->handle_locked(promise, fn, current, key, *mutex);
                    co_return;
                }),
                [](auto result) {
                });
        }
        else
        {
            co_await this->handle_locked(promise, fn, current, key, *mutex);
        }

        co_return true;
    }

    /**
     * @brief      Tries to lock the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  promise  The promise.
     * @param[in]  fn  The function.
     * @param[in]  thread  The thread.
     */
    template <typename T>
    void try_lock(const std::string&                                key,
                  std::shared_ptr<async::task_completion_source<T>> promise,
                  const async_peek_func<T>&                         fn,
                  fb::thread*                                       thread)
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

    /**
     * @brief      Locks the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  fn  The function.
     * @param[in]  trans  The trans.
     *
     * @return     The value.
     */
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

    /**
     * @brief      Tries to lock the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  fn  The function.
     *
     * @return     The value.
     */
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
    /**
     * @brief      Locks the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  fn  The function.
     * @param[in]  trans  The trans.
     *
     * @return     The value.
     */
    template <typename T>
    [[nodiscard]] async::task<T> sync(const std::string&        key,
                                      const async_wait_func<T>& fn,
                                      fb::dead_lock_detector&   trans)
    {
        auto thread  = this->_owner.threads.current();
        auto promise = std::make_shared<async::task_completion_source<T>>();

        this->lock(key, promise, fn, thread, trans);
        return promise->task();
    }

    /**
     * @brief      Locks the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  fn  The function.
     *
     * @return     The value.
     */
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

    /**
     * @brief      Locks the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  fn  The function.
     * @param[in]  trans  The trans.
     *
     * @return     The value.
     */
    template <typename T>
    T sync(const std::string& key, const sync_wait_func<T>& fn, fb::dead_lock_detector& trans)
    {
        return this->lock(key, fn, trans);
    }

    /**
     * @brief      Locks the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  fn  The function.
     *
     * @return     The value.
     */
    template <typename T>
    T sync(const std::string& key, const sync_wait_func<T>& fn)
    {
        return this->sync(key, fn, this->root);
    }

    /**
     * @brief      Locks the mutex.
     *
     * @tparam     T     The type of the value.
     *
     * @param[in]  key  The key.
     * @param[in]  fn  The function.
     *
     * @return     The value.
     */
    template <typename T>
    T try_sync(const std::string& key, const async_peek_func<T>& fn)
    {
        return async::task_completion_source<T>([this, key, &fn](auto& promise) mutable {
            auto thread = this->_owner.threads.current();
            this->try_lock(key, promise, fn, thread);
        });
    }
};

} // namespace fb

#endif // !__MUTEX_H__