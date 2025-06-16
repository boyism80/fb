#ifndef __THREAD_CONTAINER_H__
#define __THREAD_CONTAINER_H__

#include <boost/asio.hpp>
#include <fb/thread.h>
#include <fb/thread_switchable.h>
#include <map>

namespace fb {

class context;

/**
 * @brief      Container for managing multiple worker threads with task distribution.
 *
 *             This class provides a thread pool implementation that manages multiple
 *             worker threads and distributes tasks among them. It supports thread
 *             selection by index, ID, or modular arithmetic, and provides mechanisms
 *             for task enqueueing with condition checking and callback handling.
 *             Essential for multi-threaded server applications requiring load balancing.
 */
class thread_container
{
public:
    using unique_thread           = std::unique_ptr<fb::thread>;
    using unique_thread_container = std::unordered_map<std::thread::id, unique_thread>;
    using unique_id_list          = std::unique_ptr<std::thread::id[]>;

private:
    context&                _context;
    unique_thread_container _thread_container;
    unique_id_list          _keys;

public:
    std::function<void(void*)> deletor;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context to use.
     * @param      count    The number of threads to create.
     */
    thread_container(fb::context& context, uint32_t count);
    /**
     * @brief      Destroys the thread container.
     */
    ~thread_container();

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  other  The other thread container to copy from.
     */
    thread_container(const thread_container&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      other  The other thread container to move from.
     */
    thread_container(thread_container&&) = delete;

    /**
     * @brief      Assignment operator.
     *
     * @param      other  The other thread container to copy from.
     *
     * @return     The result of the assignment
     */
    thread_container& operator= (thread_container&) = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  other  The other thread container to copy from.
     *
     * @return     The result of the assignment
     */
    thread_container& operator= (const thread_container&) = delete;

public:
    /**
     * @brief      Gets the thread at the specified index.
     *
     * @param[in]  index  The index of the thread.
     *
     * @return     A pointer to the thread at the specified index.
     */
    fb::thread* at(uint8_t index) const;
    /**
     * @brief      Gets the thread with the specified thread ID.
     *
     * @param[in]  id    The ID of the thread.
     *
     * @return     A pointer to the thread with the specified ID.
     */
    fb::thread* at(std::thread::id id) const;
    /**
     * @brief      Gets a thread using modular arithmetic on the ID.
     *
     * @param[in]  id    The ID of the thread.
     *
     * @return     A pointer to the selected thread.
     */
    fb::thread* modular(uint32_t id) const;
    /**
     * @brief      Gets the current thread.
     *
     * @return     A pointer to the current thread.
     */
    fb::thread* current();
    /**
     * @brief      Gets the current thread (const version).
     *
     * @return     A const pointer to the current thread.
     */
    const fb::thread* current() const;
    /**
     * @brief      Gets the number of threads in the container.
     *
     * @return     The number of threads.
     */
    uint8_t count() const;
    /**
     * @brief      Checks if the container is empty.
     *
     * @return     True if the container is empty, false otherwise.
     */
    bool empty() const;
    /**
     * @brief      Checks if the specified index is valid.
     *
     * @param[in]  index  The index of the thread.
     *
     * @return     True if the index is valid, false otherwise.
     */
    bool valid(uint8_t index) const;
    /**
     * @brief      Checks if the specified thread pointer is valid.
     *
     * @param      thread  The thread to check.
     *
     * @return     True if the thread pointer is valid, false otherwise.
     */
    bool valid(fb::thread* thread) const;
    /**
     * @brief      Checks if the specified thread reference is valid.
     *
     * @param      thread  The thread reference to check.
     *
     * @return     True if the thread reference is valid, false otherwise.
     */
    bool valid(fb::thread& thread) const;
    /**
     * @brief      Gets the size of the container.
     *
     * @return     The size of the container.
     */
    size_t size() const;

public:
    /**
     * @brief      Enqueues a task with condition checking and callbacks.
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  condition   The condition function to check.
     * @param[in]  fn          The function to execute.
     * @param[in]  error       The error handler.
     * @param[in]  callback    The success callback.
     *
     * @tparam     ReturnType  The return type of the function.
     */
    template <typename ReturnType>
    void enqueue(thread_switchable&                                         pivot,
                 const std::function<bool(fb::thread&)>&                    condition,
                 const std::function<async::task<ReturnType>(fb::thread&)>& fn,
                 const std::function<void(std::exception&)>&                error,
                 const std::function<void(ReturnType&&)>&                   callback)
    {
        auto thread = pivot.thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->enqueue<ReturnType>(
            [=, this]() -> async::task<void> {
                if (condition(*thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread  = pivot.thread();
                auto current_thread = this->current();
                if (active_thread != current_thread)
                {
                    this->enqueue(pivot, condition, fn);
                    throw std::runtime_error("active thread not matched");
                }

                co_return co_await fn(*thread);
            },
            error,
            callback);
    }

    /**
     * @brief      Enqueues a void task with condition checking and callbacks.
     *
     * @param      pivot      The pivot object for thread selection.
     * @param[in]  condition  The condition function to check.
     * @param[in]  fn         The function to execute.
     * @param[in]  error      The error handler.
     * @param[in]  callback   The success callback.
     */
    void enqueue(thread_switchable&                                   pivot,
                 const std::function<bool(fb::thread&)>&              condition,
                 const std::function<async::task<void>(fb::thread&)>& fn,
                 const std::function<void(std::exception&)>&          error,
                 const std::function<void()>&                         callback);

    /**
     * @brief      Enqueues a task with condition checking (no callbacks).
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  condition   The condition function to check.
     * @param[in]  fn          The function to execute.
     *
     * @tparam     ReturnType  The return type of the function.
     */
    template <typename ReturnType>
    void enqueue(thread_switchable&                                         pivot,
                 const std::function<bool(fb::thread&)>&                    condition,
                 const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        return this->enqueue<ReturnType>(
            pivot,
            condition,
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    /**
     * @brief      Enqueues a void task with condition checking (no callbacks).
     *
     * @param      pivot      The pivot object for thread selection.
     * @param[in]  condition  The condition function to check.
     * @param[in]  fn         The function to execute.
     */
    void enqueue(thread_switchable&                                   pivot,
                 const std::function<bool(fb::thread&)>&              condition,
                 const std::function<async::task<void>(fb::thread&)>& fn);

    /**
     * @brief      Enqueues a task without condition checking.
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  fn          The function to execute.
     *
     * @tparam     ReturnType  The return type of the function.
     */
    template <typename ReturnType>
    void enqueue(thread_switchable& pivot, const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        return this->enqueue(
            pivot,
            []() -> bool {
                return true;
            },
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    /**
     * @brief      Enqueues a void task without condition checking.
     *
     * @param      pivot  The pivot object for thread selection.
     * @param[in]  fn     The function to execute.
     */
    void enqueue(thread_switchable& pivot, const std::function<async::task<void>(fb::thread&)>& fn);

    /**
     * @brief      Dispatches a task with condition checking and returns a future.
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  condition   The condition function to check.
     * @param[in]  fn          The function to execute.
     *
     * @tparam     ReturnType  The return type of the function.
     *
     * @return     A task that will complete when the function finishes.
     */
    template <typename ReturnType>
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                                         pivot,
                                             const std::function<bool(fb::thread&)>&                    condition,
                                             const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue(
            pivot,
            condition,
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
     * @brief      Dispatches a task with condition checking and returns a future.
     *
     * @param      pivot      The pivot object for thread selection.
     * @param[in]  condition  The condition function to check.
     * @param[in]  fn         The function to execute.
     *
     * @return     A task that will complete when the function finishes.
     */
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                                   pivot,
                                             const std::function<bool(fb::thread&)>&              condition,
                                             const std::function<async::task<void>(fb::thread&)>& fn);
    /**
     * @brief      Dispatches a task without condition checking and returns a future.
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  fn          The function to execute.
     *
     * @tparam     ReturnType  The return type of the function.
     *
     * @return     A task that will complete when the function finishes.
     */
    template <typename ReturnType>
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                                         pivot,
                                             const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue(
            pivot,
            []() -> bool {
                return true;
            },
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
     * @brief      Dispatches a void task without condition checking and returns a future.
     *
     * @param      pivot  The pivot object for thread selection.
     *
     * @return     A task that will complete when the function finishes.
     */
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                                   pivot,
                                             const std::function<async::task<void>(fb::thread&)>& fn);

    /**
     * @brief      Switches to the specified thread.
     *
     * @param      pivot  The pivot object for thread selection.
     *
     * @return     A task that completes when the switch is done.
     */
    [[nodiscard]] async::task<void> switching(thread_switchable& pivot);

    /**
     * @brief      Sets a timer with a callback.
     *
     * @param[in]  fn        The function to execute.
     * @param[in]  duration  The duration of the timer.
     */
    void settimer(const fb::timer::handle_callback_type& fn, const fb::model::timespan& duration);
    /**
     * @brief      Exits the thread.
     */
    void exit();

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  index  The index of the thread.
     *
     * @return     A pointer to the thread at the specified index.
     */
    fb::thread* operator[] (uint8_t index) const;
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  id    The ID of the thread.
     *
     * @return     A pointer to the thread with the specified ID.
     */
    fb::thread* operator[] (std::thread::id id) const;
};

} // namespace fb

#endif