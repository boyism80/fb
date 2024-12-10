#ifndef __THREAD_H__
#define __THREAD_H__

#include <boost/asio.hpp>
#include <thread>
#include <functional>
#include <map>
#include <future>
#include <atomic>
#include <fb/logger.h>
#include <fb/timer.h>
#include <fb/pqueue.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_then.h>
#include <fb/model/datetime.h>

namespace fb {

using queue_callback = std::function<void(uint8_t)>;

/**
 * @brief      This class describes a thread.
 */
class thread;
/**
 * @brief      This class describes threads.
 */
class threads;

/**
 * @brief      This class describes a thread switchable.
 */
class thread_switchable
{
protected:
    /**
     * @brief      Constructs a new instance.
     */
    thread_switchable() = default;

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~thread_switchable() = default;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual fb::thread* thread() const = 0;
    /**
     * @brief      { function_description }
     */
    void                assert_thread() const;
};

/**
 * @brief      This class describes a thread.
 */
class thread
{
public:
    using async_func_type = std::function<async::task<void>()>;
    using func_type       = std::function<void()>;

private:
    uint8_t           _index = 0;
    std::atomic<bool> _exit  = false;
    std::thread       _thread;

private:
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
     * @param[in]  index  The index
     */
    thread(uint8_t index);
    /**
     * @brief      Destroys the object.
     */
    ~thread();

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    thread(const thread&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      <unnamed>  { parameter_description }
     */
    thread(thread&&)      = delete;

    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    thread& operator= (thread&)       = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    thread& operator= (const thread&) = delete;

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     */
    void handle_thread(uint8_t index);
    /**
     * @brief      { function_description }
     */
    void handle_idle();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::thread::id id() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t         index() const;
    /**
     * @brief      { function_description }
     */
    void            exit();
    /**
     * @brief      { function_description }
     *
     * @param      value  The value
     */
    void            data(void* value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    void*           data() const;
    /**
     * @brief      { function_description }
     *
     * @tparam     ReturnType  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename ReturnType>
    ReturnType* data() const
    {
        return static_cast<ReturnType*>(this->_data);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fn          The function
     * @param[in]  duration    The duration
     * @param[in]  disposable  The disposable
     */
    void settimer(const fb::timer_callback& fn, const fb::model::timespan& duration, bool disposable = false);
    /**
     * @brief      { function_description }
     *
     * @param[in]  duration  The duration
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration);

    /**
     * @brief      { function_description }
     *
     * @param[in]  fn          The function
     * @param[in]  error       The error
     * @param[in]  callback    The callback
     *
     * @tparam     ReturnType  { description }
     */
    template <typename ReturnType>
    void enqueue(const std::function<async::task<ReturnType>()>& fn,
                 const std::function<void(std::exception&)>&     error,
                 const std::function<void(ReturnType&& value)>&  callback)
    {
        auto _ = std::lock_guard(_mutex_queue);

        this->_queue.push([=]() {
            async::awaitable_then(fn(), [&](async::awaitable_result<ReturnType> result) {
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
     * @brief      { function_description }
     *
     * @param[in]  fn        The function
     * @param[in]  error     The error
     * @param[in]  callback  The callback
     */
    void enqueue(const std::function<async::task<void>()>&   fn,
                 const std::function<void(std::exception&)>& error,
                 const std::function<void()>&                callback);

    /**
     * @brief      { function_description }
     *
     * @param[in]  fn          The function
     *
     * @tparam     ReturnType  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename ReturnType>
    async::task<ReturnType> dispatch(const std::function<async::task<ReturnType>()>& fn)
    {
        auto promise = std::make_shared<async::task_completion_source<void>>();
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
     * @brief      { function_description }
     *
     * @param[in]  fn    The function
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> dispatch(const std::function<async::task<void>()>& fn);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> switching();
};

/**
 * @brief      This class describes threads.
 */
class threads
{
public:
    using unique_thread  = std::unique_ptr<fb::thread>;
    using unique_threads = std::unordered_map<std::thread::id, unique_thread>;
    using unique_id_list = std::unique_ptr<std::thread::id[]>;

private:
    boost::asio::io_context& _context;
    unique_threads           _threads;
    unique_id_list           _keys;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     */
    threads(boost::asio::io_context& context);
    /**
     * @brief      Destroys the object.
     */
    ~threads() = default;

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    threads(const threads&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      <unnamed>  { parameter_description }
     */
    threads(threads&&)      = delete;

    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    threads& operator= (threads&)       = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    threads& operator= (const threads&) = delete;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread*       at(uint8_t index) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread*       at(std::thread::id id) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread*       modular(uint32_t id) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread*       current();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const fb::thread* current() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t           count() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool              empty() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    bool              valid(uint8_t index) const;
    /**
     * @brief      { function_description }
     *
     * @param      thread  The thread
     *
     * @return     { description_of_the_return_value }
     */
    bool              valid(fb::thread* thread) const;
    /**
     * @brief      { function_description }
     *
     * @param      thread  The thread
     *
     * @return     { description_of_the_return_value }
     */
    bool              valid(fb::thread& thread) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    size_t            size() const;

public:
    /**
     * @brief      { function_description }
     *
     * @param      pivot       The pivot
     * @param[in]  condition   The condition
     * @param[in]  fn          The function
     * @param[in]  error       The error
     * @param[in]  callback    The callback
     *
     * @tparam     ReturnType  { description }
     */
    template <typename ReturnType>
    void enqueue(thread_switchable&                              pivot,
                 const std::function<bool()>&                    condition,
                 const std::function<async::task<ReturnType>()>& fn,
                 const std::function<void(std::exception&)>&     error,
                 const std::function<void(ReturnType&&)>&        callback)
    {
        auto thread = pivot.thread();
        if (thread == nullptr)
            throw std::runtime_error("no matched thread");

        thread->enqueue<ReturnType>(
            [=, this]() -> async::task<void> {
                if (condition() == false)
                    throw std::runtime_error("condition not satisfied");

                auto active_thread  = pivot.thread();
                auto current_thread = this->current();
                if (active_thread != current_thread)
                {
                    this->enqueue(pivot, condition, fn);
                    throw std::runtime_error("active thread not matched");
                }

                co_return co_await fn();
            },
            error,
            callback);
    }

    /**
     * @brief      { function_description }
     *
     * @param      pivot      The pivot
     * @param[in]  condition  The condition
     * @param[in]  fn         The function
     * @param[in]  error      The error
     * @param[in]  callback   The callback
     */
    void enqueue(thread_switchable&                          pivot,
                 const std::function<bool()>&                condition,
                 const std::function<async::task<void>()>&   fn,
                 const std::function<void(std::exception&)>& error,
                 const std::function<void()>&                callback);

    /**
     * @brief      { function_description }
     *
     * @param      pivot       The pivot
     * @param[in]  condition   The condition
     * @param[in]  fn          The function
     *
     * @tparam     ReturnType  { description }
     */
    template <typename ReturnType>
    void enqueue(thread_switchable&                              pivot,
                 const std::function<bool()>&                    condition,
                 const std::function<async::task<ReturnType>()>& fn)
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
     * @brief      { function_description }
     *
     * @param      pivot      The pivot
     * @param[in]  condition  The condition
     * @param[in]  fn         The function
     */
    void enqueue(thread_switchable&                        pivot,
                 const std::function<bool()>&              condition,
                 const std::function<async::task<void>()>& fn);

    /**
     * @brief      { function_description }
     *
     * @param      pivot       The pivot
     * @param[in]  fn          The function
     *
     * @tparam     ReturnType  { description }
     */
    template <typename ReturnType>
    void enqueue(thread_switchable& pivot, const std::function<async::task<ReturnType>()>& fn)
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
     * @brief      { function_description }
     *
     * @param      pivot  The pivot
     * @param[in]  fn     The function
     */
    void enqueue(thread_switchable& pivot, const std::function<async::task<void>()>& fn);

    /**
     * @brief      { function_description }
     *
     * @param      pivot       The pivot
     * @param[in]  condition   The condition
     * @param[in]  fn          The function
     *
     * @tparam     ReturnType  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename ReturnType>
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                              pivot,
                                             const std::function<bool()>&                    condition,
                                             const std::function<async::task<ReturnType>()>& fn)
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
     * @brief      { function_description }
     *
     * @param      pivot      The pivot
     * @param[in]  condition  The condition
     * @param[in]  fn         The function
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                        pivot,
                                             const std::function<bool()>&              condition,
                                             const std::function<async::task<void>()>& fn);
    /**
     * @brief      { function_description }
     *
     * @param      pivot       The pivot
     * @param[in]  fn          The function
     *
     * @tparam     ReturnType  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename ReturnType>
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                              pivot,
                                             const std::function<async::task<ReturnType>()>& fn)
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
     * @brief      { function_description }
     *
     * @param      pivot  The pivot
     * @param[in]  fn     The function
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> dispatch(thread_switchable& pivot, const std::function<async::task<void>()>& fn);

    /**
     * @brief      { function_description }
     *
     * @param      pivot  The pivot
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> switching(thread_switchable& pivot);

    /**
     * @brief      { function_description }
     *
     * @param[in]  fn        The function
     * @param[in]  duration  The duration
     */
    void settimer(const fb::timer_callback& fn, const fb::model::timespan& duration);
    /**
     * @brief      { function_description }
     */
    void exit();

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  index  The index
     *
     * @return     The result of the array indexer
     */
    fb::thread* operator[] (uint8_t index) const;
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  id    The identifier
     *
     * @return     The result of the array indexer
     */
    fb::thread* operator[] (std::thread::id id) const;
};

} // namespace fb

#endif // !__THREAD_H__
