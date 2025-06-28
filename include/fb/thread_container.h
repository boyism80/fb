#ifndef __THREAD_CONTAINER_H__
#define __THREAD_CONTAINER_H__

/**
 * @file    thread_container.h
 * @brief   High-performance thread pool and task distribution system
 * @author  FB Development Team
 *
 * @details This file implements a comprehensive thread pool management system that
 *          provides efficient task distribution across multiple worker threads. The
 *          system is designed for high-performance server applications requiring
 *          load balancing, asynchronous task execution, and thread-safe operations
 *          with smart pointer integration for safe object lifetime management.
 *
 *          Key features:
 *          - Multi-threaded task distribution with intelligent load balancing
 *          - Thread selection by index, ID, or modular arithmetic for optimal distribution
 *          - Smart pointer integration with weak_ptr for safe asynchronous operations
 *          - Condition-based task enqueueing with callback handling and error management
 *          - Thread-safe operations with proper synchronization and context switching
 *          - Coroutine support for modern asynchronous programming patterns
 *          - Timer integration for scheduled task execution and periodic operations
 *          - Automatic object lifetime management in multi-threaded environments
 *          - Exception handling and error propagation across thread boundaries
 *          - Performance optimization through efficient thread pooling and reuse
 *          - Context-aware task execution with proper thread validation
 *
 * @note    This system is fundamental to the server's performance, enabling
 *          efficient handling of multiple concurrent operations while maintaining
 *          thread safety and preventing resource contention.
 */

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
 *
 *             Smart Pointer Usage:
 *             - Uses weak_ptr for safe object references in callbacks and timers
 *             - Prevents memory leaks through proper reference management
 *             - Automatically handles object lifetime in asynchronous operations
 *             - Provides type-safe access to thread_switchable objects
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
    // Thread Access Methods
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
     * @brief      Gets the thread by ID using operator[].
     *
     * @param[in]  index  The index of the thread.
     *
     * @return     A pointer to the thread.
     */
    fb::thread* operator[] (uint8_t index) const;

    /**
     * @brief      Gets the thread by thread ID using operator[].
     *
     * @param[in]  id    The thread ID.
     *
     * @return     A pointer to the thread.
     */
    fb::thread* operator[] (std::thread::id id) const;

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

    // Container Information Methods
    /**
     * @brief      Gets the number of threads in the container.
     *
     * @return     The number of threads.
     */
    uint8_t count() const;

    /**
     * @brief      Gets the size of the container.
     *
     * @return     The size.
     */
    size_t size() const;

    /**
     * @brief      Checks if the container is empty.
     *
     * @return     True if empty, false otherwise.
     */
    bool empty() const;

    // Thread Validation Methods
    /**
     * @brief      Validates if the specified index is the current thread.
     *
     * @param[in]  index  The index to validate.
     *
     * @return     True if valid, false otherwise.
     */
    bool valid(uint8_t index) const;

    /**
     * @brief      Validates if the specified thread is the current thread.
     *
     * @param[in]  thread  The thread to validate.
     *
     * @return     True if valid, false otherwise.
     */
    bool valid(const fb::thread& thread) const;

    /**
     * @brief      Validates if the specified thread pointer is the current thread.
     *
     * @param[in]  thread  The thread pointer to validate.
     *
     * @return     True if valid, false otherwise.
     */
    bool valid(thread* thread) const;

public:
    /**
     * @brief      Enqueues a task with condition checking and callbacks.
     *
     * @param      pivot       The pivot object for thread selection (as weak_ptr for safety)
     * @param[in]  condition   The condition function to check
     * @param[in]  fn          The function to execute
     * @param[in]  error       The error handler
     * @param[in]  callback    The success callback
     *
     * @tparam     ReturnType  The return type of the function
     * @tparam     T          The type of the pivot object (must inherit from thread_switchable)
     *
     * @throws     std::runtime_error if pivot object expires or thread matching fails
     *
     * @note       Uses weak_ptr to prevent circular references and handle object lifetime safely
     */
    template <typename ReturnType, typename T>
    void enqueue(std::weak_ptr<T>                                           pivot,
                 const std::function<bool(fb::thread&)>&                    condition,
                 const std::function<async::task<ReturnType>(fb::thread&)>& fn,
                 const std::function<void(std::exception&)>&                error,
                 const std::function<void(ReturnType&&)>&                   callback)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto thread = pivot->thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->template enqueue<ReturnType>(
            [=, this]() -> async::task<void> {
                auto shared = pivot.lock();
                if (shared == nullptr)
                    throw std::runtime_error("pivot object is expired");

                if (condition(*thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread  = shared->thread();
                auto current_thread = this->current();
                if (active_thread != current_thread)
                {
                    this->enqueue(shared.week_from_this(), condition, fn);
                    throw std::runtime_error("active thread not matched");
                }

                co_return co_await fn(*thread);
            },
            error,
            callback);
    }

    /**
     * @brief      Enqueues a task with condition checking (no callbacks).
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  condition   The condition function to check.
     * @param[in]  fn          The function to execute.
     *
     * @tparam     ReturnType  The return type of the function.
     */
    template <typename ReturnType, typename T>
    void enqueue(std::weak_ptr<T>                                           pivot,
                 const std::function<bool(fb::thread&)>&                    condition,
                 const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        return this->enqueue<ReturnType, T>(
            pivot,
            condition,
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
    }

    /**
     * @brief      Enqueues a task without condition checking.
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  fn          The function to execute.
     *
     * @tparam     ReturnType  The return type of the function.
     */
    template <typename ReturnType, typename T>
    void enqueue(std::weak_ptr<T> pivot, const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        return this->enqueue<ReturnType, T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [](std::exception& e) {
            },
            [](ReturnType&& value) {
            });
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
    template <typename T>
    void enqueue(std::weak_ptr<T>                            pivot,
                 const std::function<bool(fb::thread&)>&     condition,
                 const thread::handle_func_type<void>&       fn,
                 const std::function<void(std::exception&)>& error,
                 const std::function<void()>&                callback)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto thread = shared->thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->enqueue(
            [=, this](auto& thread) -> async::task<void> {
                auto shared = pivot.lock();
                if (shared == nullptr)
                    throw std::runtime_error("pivot object is expired");

                if (condition(thread) == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread = shared->thread();
                if (active_thread != &thread)
                {
                    this->enqueue(pivot, condition, fn);
                }
                else
                {
                    co_await fn(*active_thread);
                }
            },
            error,
            callback);
    }

    /**
     * @brief      Enqueues a void task with condition checking (no callbacks).
     *
     * @param      pivot      The pivot object for thread selection.
     * @param[in]  condition  The condition function to check.
     * @param[in]  fn         The function to execute.
     */
    template <typename T>
    void enqueue(std::weak_ptr<T>                        pivot,
                 const std::function<bool(fb::thread&)>& condition,
                 const thread::handle_func_type<void>&   fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        return this->enqueue<T>(
            pivot,
            condition,
            fn,
            [](std::exception& e) {
            },
            []() {
            });
    }

    /**
     * @brief      Enqueues a void task without condition checking.
     *
     * @param      pivot  The pivot object for thread selection.
     * @param[in]  fn     The function to execute.
     */
    template <typename T>
    void enqueue(std::weak_ptr<T> pivot, const thread::handle_func_type<void>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        return this->enqueue<T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [](std::exception& e) {
            },
            []() {
            });
    }

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
    template <typename ReturnType, typename T>
    [[nodiscard]] async::task<ReturnType> dispatch(std::weak_ptr<T>                                           pivot,
                                                   const std::function<bool(fb::thread&)>&                    condition,
                                                   const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto target_thread = shared->thread();
        if (target_thread == nullptr)
            throw std::runtime_error("no matched thread");

        if (target_thread->id() == std::this_thread::get_id())
        {
            if (condition(*target_thread) == false)
                throw std::runtime_error("condition not satisfied");

            co_return co_await fn(*target_thread);
        }

        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue<ReturnType, T>(
            pivot,
            condition,
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        co_return co_await promise->task();
    }

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
    template <typename ReturnType, typename T>
    [[nodiscard]] async::task<ReturnType> dispatch(std::weak_ptr<T>                                           pivot,
                                                   const std::function<async::task<ReturnType>(fb::thread&)>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto target_thread = shared->thread();
        if (target_thread == nullptr)
            throw std::runtime_error("no matched thread");

        if (target_thread->id() == std::this_thread::get_id())
        {
            co_return co_await fn(*target_thread);
        }

        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue<ReturnType, T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(value);
            });
        co_return co_await promise->task();
    }

    /**
     * @brief      Dispatches a void task with condition checking and returns a future.
     *
     * @param      pivot       The pivot object for thread selection.
     * @param[in]  condition   The condition function to check.
     * @param[in]  fn          The function to execute.
     *
     * @return     A task that will complete when the function finishes.
     */
    template <typename T>
    async::task<void> dispatch(std::weak_ptr<T>                        pivot,
                               const std::function<bool(fb::thread&)>& condition,
                               const thread::handle_func_type<void>&   fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto target_thread = shared->thread();
        if (target_thread == nullptr)
            throw std::runtime_error("no matched thread");

        if (target_thread->id() == std::this_thread::get_id())
        {
            if (condition(*target_thread) == false)
                throw std::runtime_error("condition not satisfied");

            co_await fn(*target_thread);
            co_return;
        }

        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue<T>(
            pivot,
            condition,
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise]() {
                promise->set_value();
            });
        co_await promise->task();
    }

    /**
     * @brief      Dispatches a void task without condition checking and returns a future.
     *
     * @param      pivot  The pivot object for thread selection.
     * @param[in]  fn     The function to execute.
     *
     * @return     A task that will complete when the function finishes.
     */
    template <typename T>
    async::task<void> dispatch(std::weak_ptr<T> pivot, const thread::handle_func_type<void>& fn)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        auto shared = pivot.lock();
        if (shared == nullptr)
            throw std::runtime_error("pivot object is expired");

        auto target_thread = shared->thread();
        if (target_thread == nullptr)
            throw std::runtime_error("no matched thread");

        if (target_thread->id() == std::this_thread::get_id())
        {
            co_await fn(*target_thread);
            co_return;
        }

        auto promise = std::make_shared<async::task_completion_source<void>>();
        this->enqueue<T>(
            pivot,
            [](auto& thread) -> bool {
                return true;
            },
            fn,
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise]() {
                promise->set_value();
            });
        co_await promise->task();
    }

    /**
     * @brief      Switches to the specified thread using a weak_ptr reference.
     *
     * @param      pivot  The pivot object for thread selection (as weak_ptr)
     */
    template <typename T>
    async::task<void> switching(std::weak_ptr<T> pivot)
    {
        static_assert(std::is_base_of_v<thread_switchable, T>, "T must be a thread_switchable");

        co_await this->dispatch(pivot, [](auto& thread) -> async::task<void> {
            co_return;
        });
    }

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
};

} // namespace fb

#endif // !__THREAD_CONTAINER_H__