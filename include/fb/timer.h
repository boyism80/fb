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

/**
 * @brief      This class describes a timer.
 */
class timer
{
public:
    friend class fb::thread;

public:
    /**
     * @brief      The type of the callback function.
     */
    using handle_callback_type = std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>;

public:
    const handle_callback_type fn;
    const fb::model::timespan  duration;
    const bool                 disposable = false;
    fb::model::datetime        begin;

private:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  fn        The callback function.
     * @param[in]  duration  The duration of the timer.
     * @param[in]  disposable  Whether the timer is disposable.
     */
    timer(const handle_callback_type& fn, const fb::model::timespan& duration, bool disposable);
    /**
     * @brief      Copy constructor (deleted).
     *
     * @param[in]  other  The other timer to copy from.
     */
    timer(const timer&) = delete;
    /**
     * @brief      Move constructor (deleted).
     *
     * @param[in]  other  The other timer to move from.
     */
    timer(timer&&) = delete;

    /**
     * @brief      Assignment operator (deleted).
     *
     * @param[in]  other  The other timer to copy from.
     */
    timer& operator= (timer&) = delete;

    /**
     * @brief      Assignment operator (deleted).
     *
     * @param[in]  other  The other timer to copy from.
     */
    timer& operator= (const timer&) = delete;

public:
    /**
     * @brief      Destroys the timer.
     */
    ~timer()
    { }
};

} // namespace fb

#endif