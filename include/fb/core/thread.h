#ifndef __THREAD_H__
#define __THREAD_H__

#include <boost/asio.hpp>
#include <thread>
#include <functional>
#include <map>
#include <future>
#include <atomic>
#include <fb/core/coroutine.h>
#include <fb/core/logger.h>
#include <fb/core/timer.h>
#include <fb/core/pqueue.h>

#define MUTEX_GUARD(x) auto _ = std::lock_guard(x);

namespace fb {

using queue_callback  = std::function<void(uint8_t)>;

class thread
{
public:
    class task
    {
    public:
        using func_type = std::function<fb::task<void>()>;
        using optional_awaiter = std::optional<fb::awaiter<void>>;

    public:
        func_type               func;
        optional_awaiter        awaiter;

    public:
        task(const func_type& func);
        task(const func_type& func, fb::awaiter<void>&& awaiter);
        task(const task&) = delete;
        task(task&&) noexcept;
        ~task();

    public:
        void operator = (const task&) = delete;
        void operator = (task&& r) noexcept;
    };

private:
    uint8_t                                         _index = 0;
    std::atomic<bool>                               _exit  = false;
    std::thread                                     _thread;

private:
    std::vector<std::unique_ptr<timer>>             _timers;
    std::recursive_mutex                            _mutex_timer;

private:
    fb::queue<fb::thread::task>                     _queue;
    std::vector<fb::task<void>>                     _tasks;

public:
    thread(uint8_t index);
    ~thread();

    thread(const thread&) = delete;
    thread(thread&&) = delete;

    thread& operator = (thread&) = delete;
    thread& operator = (const thread&) = delete;

private:
    void                                            handle_thread(uint8_t index);
    void                                            handle_idle();

public:
    std::thread::id                                 id() const;
    uint8_t                                         index() const;
    void                                            exit();

public:
    fb::awaiter<void>                               dispatch(const std::function<fb::task<void>()>& fn, const std::chrono::steady_clock::duration& delay = 0s, int priority = 0);
    void                                            post(const std::function<fb::task<void>()>& fn, const std::chrono::steady_clock::duration& delay = 0s, int priority = 0);
    fb::awaiter<void>                               dispatch(uint32_t priority = 0);
    void                                            settimer(const fb::timer_callback& fn, const std::chrono::steady_clock::duration& duration, bool disposable = false);
    fb::awaiter<void>                               sleep(const std::chrono::steady_clock::duration& duration);

public:
    static std::chrono::steady_clock::duration      now();
};

class threads
{
public:
    using unique_thread  = std::unique_ptr<fb::thread>;
    using unique_threads = std::map<std::thread::id, unique_thread>;
    using unique_id_list = std::unique_ptr<std::thread::id[]>;

private:
    boost::asio::io_context&                        _context;
    unique_threads                                  _threads;
    unique_id_list                                  _keys;

public:
    threads(boost::asio::io_context& context, uint8_t count);
    ~threads() = default;

    threads(const threads&) = delete;
    threads(threads&&) = delete;

    threads& operator = (threads&) = delete;
    threads& operator = (const threads&) = delete;

public:
    fb::thread*                                     at(uint8_t index) const;
    fb::thread*                                     at(std::thread::id id) const;
    fb::thread*                                     current();
    const fb::thread*                               current() const;
    uint8_t                                         count() const;
    bool                                            empty() const;
    bool                                            valid(uint8_t index) const;
    bool                                            valid(fb::thread* thread) const;
    bool                                            valid(fb::thread& thread) const;
    size_t                                          size() const;

public:
    fb::task<void>                                  dispatch(const std::function<fb::task<void>()>& fn, const std::chrono::steady_clock::duration& delay);
    void                                            settimer(const fb::timer_callback& fn, const std::chrono::steady_clock::duration& duration);
    void                                            exit();

public:
    fb::thread*                                     operator [] (uint8_t index) const;
    fb::thread*                                     operator [] (std::thread::id id) const;
};

}

#endif // !__THREAD_H__
