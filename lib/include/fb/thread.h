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

namespace fb {

/**
 * @brief      This class describes a thread.
 */
class thread
{
public:
    template <typename ReturnType>
    using handle_func_type  = std::function<async::task<ReturnType>(fb::thread&)>;
    using handle_error_type = std::function<void(std::exception&)>;

private:
    uint8_t           _index = 0;
    std::atomic<bool> _exit  = false;
    std::thread       _thread;

private:
    std::unordered_set<void*>           _ptrs;
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
    thread(thread&&) = delete;

    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    thread& operator= (thread&) = delete;
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

    /**
     * @brief      { function_description }
     */
    void assert_exec() const;

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
    uint8_t index() const;
    /**
     * @brief      { function_description }
     */
    void exit();
    /**
     * @brief      { function_description }
     *
     * @param      value  The value
     *
     * @tparam     T      { description }
     */
    template <typename T>
    void data(T* value)
    {
        this->assert_exec();
        this->_data = static_cast<void*>(value);
    }
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
        this->assert_exec();
        return static_cast<ReturnType*>(this->_data);
    }

    /**
     * @brief      Pushes a pointer.
     *
     * @param      ptr   The pointer
     */
    void push_ptr(void* ptr);

    /**
     * @brief      { function_description }
     *
     * @param      ptr   The pointer
     */
    void pop_ptr(void* ptr);

    /**
     * @brief      { function_description }
     *
     * @param      ptr   The pointer
     */
    void assert_ptr(void* ptr) const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fn          The function
     * @param[in]  duration    The duration
     * @param[in]  disposable  The disposable
     */
    void settimer(const fb::timer::handle_callback_type& fn,
                  const fb::model::timespan&             duration,
                  bool                                   disposable = false);
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
     * @brief      { function_description }
     *
     * @param[in]  fn        The function
     * @param[in]  error     The error
     * @param[in]  callback  The callback
     */
    void enqueue(const handle_func_type<void>& fn,
                 const handle_error_type&      error,
                 const std::function<void()>&  callback);

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
    async::task<ReturnType> dispatch(const handle_func_type<ReturnType>& fn)
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
    [[nodiscard]] async::task<void> dispatch(const handle_func_type<void>& fn);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> switching();
};

} // namespace fb

#endif // !__THREAD_H__
