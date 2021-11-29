#ifndef __THREAD_H__
#define __THREAD_H__

#include <boost/asio.hpp>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <map>
#include <future>
#include <atomic>

#define MUTEX_GUARD(x) auto __gd = std::lock_guard(x);

using namespace std::chrono_literals;

namespace fb {

using thread_callback = std::function<void(std::chrono::steady_clock::duration, std::thread::id)>;
using queue_callback  = std::function<void(uint8_t)>;

class thread;

class timer
{
public:
    friend class fb::thread;

public:
    const fb::thread_callback                       fn;
    const std::chrono::steady_clock::time_point     begin;
    const std::chrono::steady_clock::duration       duration;

private:
    timer(const fb::thread_callback& fn, std::chrono::steady_clock::duration duration);
    timer(const timer&) = delete;
    timer(timer&&) = delete;

    timer& operator = (timer&) = delete;
    timer& operator = (const timer&) = delete;

public:
    ~timer();
};

class queue : private std::queue<fb::queue_callback>
{
private:
    std::mutex                                      _mutex;

public:
    queue() = default;
    ~queue() = default;

    queue(const queue&) = delete;
    queue(queue&&) = delete;

    queue& operator = (queue&) = delete;
    queue& operator = (const queue&) = delete;

public:
    bool                                            empty();
    void                                            enqueue(fb::queue_callback fn);
    fb::queue_callback                              dequeue();
    bool                                            dequeue(fb::queue_callback& fn);
    void                                            flush(uint8_t index);
};


class thread
{
private:
    uint8_t                                         _index = 0;
    std::atomic<bool>                               _exit  = false;
    std::thread                                     _thread;

private:
    std::vector<std::shared_ptr<timer>>             _timers;
    std::mutex                                      _mutex_timer;

public:
    fb::queue                                       queue, precedence;

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
    void                                            dispatch(const std::function<void()>& fn, const std::chrono::steady_clock::duration& duration);
    void                                            settimer(fb::thread_callback fn, const std::chrono::steady_clock::duration& duration);

public:
    static std::chrono::steady_clock::duration      now();
};

class async
{
private:
    static std::unique_ptr<async>                   _ist;

private:
    std::thread                                     _async_thread;
    std::vector<std::future<void>>                  _futures;
    bool                                            _exit;
    std::mutex                                      _async_mutex;

public:
    async();
    ~async();

private:
    void                                            _launch(const std::function<void()>& fn);
    void                                            async_handler();
    static async*                                   get();

public:
    static void                                     launch(const std::function<void()>& fn);
    static void                                     exit();
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
    void                                            dispatch(const std::function<void()>& fn, const std::chrono::steady_clock::duration& duration, bool main = false);
    void                                            settimer(fb::thread_callback fn, const std::chrono::steady_clock::duration& duration);
    void                                            exit();

public:
    fb::thread*                                     operator [] (uint8_t index) const;
    fb::thread*                                     operator [] (std::thread::id id) const;
};

}

#endif // !__THREAD_H__
