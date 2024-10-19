#ifndef __TIMER_H__
#define __TIMER_H__

#include <chrono>
#include <fb/model/datetime.h>

using namespace std::chrono_literals;

namespace fb {

class thread;

using timer_callback = std::function<void(const fb::model::datetime&, std::thread::id)>;

class timer
{
public:
    friend class fb::thread;

public:
    const fb::timer_callback                        fn;
    const fb::model::datetime                       begin;
    const fb::model::timespan                       duration;
    const bool                                      disposable = false;

private:
    timer(const fb::timer_callback& fn, const fb::model::timespan& duration, bool disposable) : fn(fn), duration(duration), disposable(disposable)
    {}
    timer(const timer&) = delete;
    timer(timer&&) = delete;

    timer& operator = (timer&) = delete;
    timer& operator = (const timer&) = delete;

public:
    ~timer() = default;
};

}

#endif