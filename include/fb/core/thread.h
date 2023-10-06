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
#include <fb/core/coroutine.h>

#define MUTEX_GUARD(x) auto __gd = std::lock_guard<std::mutex>(x);

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
    const std::chrono::steady_clock::time_point     begin = std::chrono::steady_clock::now();
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

template <typename T>
class queue : private std::queue<T>
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
    bool                                            empty()
    {   MUTEX_GUARD(this->_mutex)

        return std::queue<T>::empty();
    }
    void                                            enqueue(T&& fn)
    {   MUTEX_GUARD(this->_mutex)

        this->push(fn);
    }
    T                                               dequeue()
    {   MUTEX_GUARD(this->_mutex)

        auto& fn = this->front();
        this->pop();
        return std::move(fn);
    }
    bool                                            dequeue(T& fn)
    {   MUTEX_GUARD(this->_mutex)

        auto empty = std::queue<T>::empty();
        if(empty)
            return false;

        fn = this->front();
        this->pop();
        return true;
    }
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

private:
    fb::queue<fb::queue_callback>                   _queue, _precedence;
    fb::queue<fb::awaitable<void>*>                 _dispatch_awaitable_queue, _precedence_awaitable_queue;

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
    void                                            dispatch(fb::queue_callback&& fn, bool precedence = false);
    fb::awaitable<void>                             dispatch();
    fb::awaitable<void>                             precedence();

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
    void                                            dispatch(const std::function<void()>& fn, const std::chrono::steady_clock::duration& duration, bool main = false);
    void                                            settimer(fb::thread_callback fn, const std::chrono::steady_clock::duration& duration);
    void                                            exit();

public:
    fb::thread*                                     operator [] (uint8_t index) const;
    fb::thread*                                     operator [] (std::thread::id id) const;
};

}

#endif // !__THREAD_H__
