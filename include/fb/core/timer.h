#ifndef __TIMER_H__
#define __TIMER_H__

#include <chrono>

using namespace std::chrono_literals;

namespace fb {

class thread;

using timer_callback = std::function<void(std::chrono::steady_clock::duration, std::thread::id)>;

class timer
{
public:
    friend class fb::thread;

public:
    const fb::timer_callback                        fn;
    const std::chrono::steady_clock::time_point     begin = std::chrono::steady_clock::now();
    const std::chrono::steady_clock::duration       duration;

private:
    timer(const fb::timer_callback& fn, std::chrono::steady_clock::duration duration) : fn(fn), duration(duration)
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