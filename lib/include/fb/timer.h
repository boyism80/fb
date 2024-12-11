#ifndef __TIMER_H__
#define __TIMER_H__

#include <fb/model/datetime.h>
#include <chrono>
#include <functional>
#include <async/task.h>
#include <thread>

using namespace std::chrono_literals;

namespace fb {

class thread;

class timer
{
public:
    friend class fb::thread;

public:
    using handle_callback_type = std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>;

public:
    const handle_callback_type fn;
    const fb::model::datetime  begin;
    const fb::model::timespan  duration;
    const bool                 disposable = false;

private:
    timer(const handle_callback_type& fn, const fb::model::timespan& duration, bool disposable);
    timer(const timer&) = delete;
    timer(timer&&)      = delete;

    timer& operator= (timer&)       = delete;
    timer& operator= (const timer&) = delete;

public:
    ~timer() = default;
};

} // namespace fb

#endif