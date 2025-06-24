#ifndef __THREAD_H__
#define __THREAD_H__

/**
 * @file    thread.h
 * @brief   Managed thread system with asynchronous task execution and smart pointer integration
 * @author  FB Development Team
 *
 * @details This file implements the core thread management system for the FB 2D MMORPG
 *          game server. It provides managed threads that can execute asynchronous tasks,
 *          handle timers, and safely manage thread_switchable objects through smart
 *          pointer integration.
 *
 *          Key features:
 *          - Asynchronous task execution with queue-based task management
 *          - Comprehensive timer system with disposable and recurring timers
 *          - Smart pointer integration for safe thread_switchable object handling
 *          - Lua scripting integration for thread-based game logic
 *          - Thread-safe task enqueueing with proper synchronization
 *          - Automatic resource cleanup and proper thread lifecycle management
 *          - Thread-local data storage for context-specific information
 *          - Exception handling and error callback mechanisms
 *          - Sleep functionality for timed operations and delays
 *          - Integration with async/await coroutine patterns
 *
 * @note    This thread system is specifically designed for game server architecture
 *          where thread safety, performance, and proper resource management are critical.
 */

#include <thread>
#include <functional>
#include <future>
#include <atomic>
#include <queue>
#include <fb/logger.h>
#include <fb/timer.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_then.h>
#include <fb/model/datetime.h>
#include <unordered_set>
#include <fb/lua.h>

namespace fb {

class thread_container;

/**
 * @brief      Thread class for managing asynchronous task execution.
 *
 *             This class represents a managed thread that can execute tasks asynchronously.
 *             It inherits from luable to allow Lua script interaction and provides
 *             comprehensive task queueing, timer management, and thread switching capabilities.
 *             The class is designed to work seamlessly with the smart pointer system,
 *             particularly for handling thread_switchable objects.
 *
 *             Smart Pointer Integration:
 *             - Safely manages thread_switchable objects through weak_ptr references
 *             - Prevents memory leaks in asynchronous operations
 *             - Provides type-safe task dispatching with proper lifetime management
 *             - Integrates with the Lua binding system for safe object handling
 */
class thread : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

    friend class thread_container;

public:
    template <typename ReturnType>
    using handle_func_type  = std::function<async::task<ReturnType>(fb::thread&)>;
    using handle_error_type = std::function<void(std::exception&)>;

private:
    uint8_t           _index = 0;
    std::atomic<bool> _exit  = false;
    std::thread       _thread;

private:
    std::unordered_set<const void*>     _ptrs;
    std::vector<std::unique_ptr<timer>> _timers;
    std::recursive_mutex                _mutex_timer;
    void*                               _data = nullptr;

private:
    std::queue<std::function<void()>> _queue;
    std::mutex                        _mutex_queue;

public:
    /**
     * @brief      Constructs a new thread with the specified index.
     *
     *             Creates and starts a new managed thread for asynchronous execution.
     *             The thread will begin processing tasks and timers immediately.
     *             Thread safety is ensured through proper mutex usage and smart pointer
     *             management for thread_switchable objects.
     *
     * @param[in]  index  The unique identifier for this thread within the thread pool
     */
    thread(uint8_t index);

    /**
     * @brief      Destroys the thread and waits for completion.
     *
     *             Signals the thread to exit, waits for all pending tasks to complete,
     *             and cleans up all associated resources including timers and queued tasks.
     *             Ensures proper cleanup of any remaining weak_ptr references to
     *             thread_switchable objects.
     */
    ~thread();

    /**
     * @brief      Move constructor (deleted).
     *
     *             Thread objects cannot be moved as they maintain unique thread IDs
     *             and resource ownership.
     *
     * @param      other  The other thread to move from
     */
    thread(thread&&) = delete;

    /**
     * @brief      Assignment operator (deleted).
     *
     *             Thread objects cannot be copied or assigned as they maintain
     *             unique thread IDs and resource ownership.
     *
     * @param      other  The other thread to copy from
     *
     * @return     Reference to this thread
     */
    thread& operator= (thread&) = delete;

    /**
     * @brief      Copy assignment operator (deleted).
     *
     * @param[in]  other  The other thread to copy from.
     *
     * @return     Reference to this thread.
     */
    thread& operator= (const thread&) = delete;

private:
    /**
     * @brief      Handles thread execution with the specified index.
     *
     * @param[in]  index  The index of the thread.
     */
    void handle_thread(uint8_t index);
    /**
     * @brief      Handles idle state processing.
     */
    void handle_idle();

    /**
     * @brief      Asserts that the current thread is the execution thread.
     */
    void assert_exec() const;

public:
    /**
     * @brief      Gets the thread ID.
     *
     * @return     The thread ID.
     */
    std::thread::id id() const;
    /**
     * @brief      Gets the thread index.
     *
     * @return     The thread index.
     */
    uint8_t index() const;
    /**
     * @brief      Exits the thread.
     */
    void exit();
    /**
     * @brief      Sets thread-local data.
     *
     * @param      value  The value to store.
     *
     * @tparam     T      The type of the data.
     */
    template <typename T>
    void data(T* value)
    {
        this->assert_exec();
        this->_data = static_cast<void*>(value);
    }
    /**
     * @brief      Gets thread-local data.
     *
     * @tparam     ReturnType  The type of the data to retrieve.
     *
     * @return     A pointer to the thread-local data.
     */
    template <typename ReturnType>
    ReturnType* data() const
    {
        this->assert_exec();
        return static_cast<ReturnType*>(this->_data);
    }

public:
    /**
     * @brief      Sets a timer with the specified callback and duration.
     *
     * @param[in]  fn          The callback function.
     * @param[in]  duration    The timer duration.
     * @param[in]  disposable  Whether the timer is disposable.
     */
    void settimer(const fb::timer::handle_callback_type& fn,
                  const fb::model::timespan&             duration,
                  bool                                   disposable = false);
    /**
     * @brief      Sleeps for the specified duration.
     *
     * @param[in]  duration  The duration to sleep.
     */
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration);

    /**
     * @brief      Enqueues a task with callback and error handling.
     *
     * @param[in]  fn          The function to execute.
     * @param[in]  error       The error handler.
     * @param[in]  callback    The success callback.
     *
     * @tparam     ReturnType  The return type of the function.
     */
    template <typename ReturnType>
    void enqueue(const handle_func_type<ReturnType>&      fn,
                 const handle_error_type&                 error,
                 const std::function<void(ReturnType&&)>& callback)
    {
        auto _ = std::lock_guard(_mutex_queue);

        this->_queue.push([=, this]() {
            async::awaitable_then(fn(*this), [&](async::awaitable_result<ReturnType> result) {
                try
                {
                    callback(result());
                }
                catch (std::exception& e)
                {
                    error(e);
                }
                catch (...)
                {
                    try
                    {
                        std::rethrow_exception(std::current_exception());
                    }
                    catch (std::exception& e)
                    {
                        error(e);
                    }
                }
            });
        });
    }

    /**
     * @brief      Enqueues a void task with callback and error handling.
     *
     * @param[in]  fn        The function to execute.
     * @param[in]  error     The error handler.
     * @param[in]  callback  The success callback.
     */
    void enqueue(const handle_func_type<void>& fn,
                 const handle_error_type&      error,
                 const std::function<void()>&  callback);

    /**
     * @brief      Dispatches a task and returns a future result.
     *
     * @param[in]  fn          The function to execute.
     *
     * @tparam     ReturnType  The return type of the function.
     *
     * @return     A task that will complete with the function result.
     */
    template <typename ReturnType>
    async::task<ReturnType> dispatch(const handle_func_type<ReturnType>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue<ReturnType>(
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        return promise->task();
    }

    /**
     * @brief      Dispatches a void task and returns a future.
     *
     * @param[in]  fn    The function to execute.
     *
     * @return     A task that will complete when the function finishes.
     */
    [[nodiscard]] async::task<void> dispatch(const handle_func_type<void>& fn);

    /**
     * @brief      Switches to this thread context.
     *
     * @return     A task that completes when the switch is done.
     */
    [[nodiscard]] async::task<void> switching();
};

} // namespace fb

#endif // !__THREAD_H__
