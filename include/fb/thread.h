#ifndef __THREAD_H__
#define __THREAD_H__

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
 * @brief      This class describes a thread.
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
     * @brief      Constructs a new instance.
     *
     * @param[in]  index  The index of the thread.
     */
    thread(uint8_t index);
    /**
     * @brief      Destroys the thread.
     */
    ~thread();

    /**
     * @brief      Move constructor (deleted).
     *
     * @param      other  The other thread to move from.
     */
    thread(thread&&) = delete;

    /**
     * @brief      Assignment operator (deleted).
     *
     * @param      other  The other thread to copy from.
     *
     * @return     Reference to this thread.
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
