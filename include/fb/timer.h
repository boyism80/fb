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
 * @brief      High-precision timer for scheduling asynchronous callbacks.
 *
 *             This class provides a timer mechanism that can execute callback functions
 *             at specified intervals or after a delay. Timers can be configured as
 *             disposable (one-shot) or repeating, and integrate with the thread system
 *             for asynchronous execution.
 */
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
    /**
     * @brief      Callback function type for timer events.
     *
     *             The callback receives the current datetime and the thread ID
     *             where the timer is executing.
     */
    using handle_callback_type = std::function<async::task<void>(const fb::model::datetime&, std::thread::id)>;

private:
    bool _canceled = false;

public:
    const handle_callback_type fn;       ///< The callback function to execute
    const fb::model::timespan  duration; ///< The timer interval or delay
    const repeat_type          repeat;   ///< The timer repeat type
    fb::model::datetime        begin;    ///< The start time of the timer

private:
    /**
     * @brief      Constructs a new timer with the specified parameters.
     *
     *             Creates a timer that will execute the given callback function
     *             after the specified duration. The timer can be configured as
     *             once (executes once) or repeat.
     *
     * @param[in]  fn          The callback function to execute when the timer fires
     * @param[in]  duration    The time interval for the timer
     * @param[in]  repeat      The timer repeat type
     */
    timer(const handle_callback_type& fn, const fb::model::timespan& duration, repeat_type repeat);

    /**
     * @brief      Copy constructor (deleted).
     *
     *             Timers cannot be copied to prevent resource management issues.
     */
    timer(const timer&) = delete;

    /**
     * @brief      Move constructor (deleted).
     *
     *             Timers cannot be moved to maintain thread safety and resource integrity.
     */
    timer(timer&&) = delete;

    /**
     * @brief      Copy assignment operator (deleted).
     *
     *             Timers cannot be assigned to prevent resource management issues.
     */
    timer& operator= (timer&) = delete;

    /**
     * @brief      Copy assignment operator (deleted).
     *
     *             Timers cannot be assigned to prevent resource management issues.
     */
    timer& operator= (const timer&) = delete;

public:
    /**
     * @brief      Destroys the timer and cleans up resources.
     *
     *             Ensures proper cleanup of timer resources when the timer
     *             is destroyed or goes out of scope.
     */
    ~timer()
    { }

public:
    /**
     * @brief      Cancels the timer.
     *
     *             Prevents the timer from executing its callback function.
     */
    void cancel()
    {
        this->_canceled = true;
    }

    /**
     * @brief      Checks if the timer has been canceled.
     *
     * @return     True if the timer has been canceled, false otherwise.
     */
    bool canceled() const
    {
        return this->_canceled;
    }
};

} // namespace fb

#endif