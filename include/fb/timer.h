#ifndef __TIMER_H__
#define __TIMER_H__

#include <fb/model/datetime.h>
#include <chrono>
#include <functional>
#include <async/task.h>
#include <thread>

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
    bool _canceled = false;

public:
    const handle_callback_type fn;
    const fb::model::timespan  duration;
    const repeat_type          repeat;
    fb::model::datetime        begin;

private:
    timer(const handle_callback_type& fn, const fb::model::timespan& duration, repeat_type repeat);
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
};

} // namespace fb

#endif