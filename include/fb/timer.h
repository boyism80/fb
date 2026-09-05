#ifndef __TIMER_H__
#define __TIMER_H__

#include <fb/model/datetime.h>
#include <chrono>
#include <functional>
#include <async/propagation.h>
#include <async/task.h>
#include <thread>
#include <atomic>

namespace fb {

using namespace std::chrono_literals;

class thread;

class timer
{
public:
    enum class repeat_type : uint8_t
    {
        once   = 0,
        repeat = 1,
    };

public:
    friend class fb::thread;

public:
    using handle_callback_type = std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>;

private:
    bool              _canceled = false;
    std::atomic<bool> _running{false};

public:
    const handle_callback_type      fn;
    const fb::model::timespan       duration;
    const repeat_type               repeat;
    const async::propagation::token context;
    fb::model::datetime             begin;

private:
    timer(const handle_callback_type& fn,
          const fb::model::timespan&  duration,
          repeat_type                 repeat,
          async::propagation::token   context);
    timer(const timer&)             = delete;
    timer(timer&&)                  = delete;
    timer& operator= (timer&)       = delete;
    timer& operator= (const timer&) = delete;

public:
    ~timer() = default;

public:
    void cancel()
    {
        this->_canceled = true;
    }

    bool canceled() const
    {
        return this->_canceled;
    }

    bool running() const
    {
        return this->_running.load(std::memory_order_acquire);
    }

    bool try_begin_run()
    {
        bool expected = false;
        return this->_running.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
    }

    void end_run()
    {
        this->_running.store(false, std::memory_order_release);
    }
};

} // namespace fb

#endif