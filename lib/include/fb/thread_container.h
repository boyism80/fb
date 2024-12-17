#ifndef __THREAD_CONTAINER_H__
#define __THREAD_CONTAINER_H__

#include <boost/asio.hpp>
#include <fb/thread.h>
#include <fb/thread_switchable.h>
#include <map>

namespace fb {

/**
 * @brief      This class describes thread_container.
 */
class thread_container
{
public:
    using unique_thread           = std::unique_ptr<fb::thread>;
    using unique_thread_container = std::unordered_map<std::thread::id, unique_thread>;
    using unique_id_list          = std::unique_ptr<std::thread::id[]>;

private:
    boost::asio::io_context& _context;
    unique_thread_container  _thread_container;
    unique_id_list           _keys;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     */
    thread_container(boost::asio::io_context& context);
    /**
     * @brief      Destroys the object.
     */
    ~thread_container() = default;

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    thread_container(const thread_container&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      <unnamed>  { parameter_description }
     */
    thread_container(thread_container&&) = delete;

    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    thread_container& operator= (thread_container&) = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    thread_container& operator= (const thread_container&) = delete;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* at(uint8_t index) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* at(std::thread::id id) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* modular(uint32_t id) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* current();
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
    uint8_t count() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool empty() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    bool valid(uint8_t index) const;
    /**
     * @brief      { function_description }
     *
     * @param      thread  The thread
     *
     * @return     { description_of_the_return_value }
     */
    bool valid(fb::thread* thread) const;
    /**
     * @brief      { function_description }
     *
     * @param      thread  The thread
     *
     * @return     { description_of_the_return_value }
     */
    bool valid(fb::thread& thread) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    size_t size() const;

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
     * @brief      { function_description }
     *
     * @param      pivot      The pivot
     * @param[in]  condition  The condition
     * @param[in]  fn         The function
     * @param[in]  error      The error
     * @param[in]  callback   The callback
     */
    void enqueue(thread_switchable&                                   pivot,
                 const std::function<bool(fb::thread&)>&              condition,
                 const std::function<async::task<void>(fb::thread&)>& fn,
                 const std::function<void(std::exception&)>&          error,
                 const std::function<void()>&                         callback);

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
     * @brief      { function_description }
     *
     * @param      pivot      The pivot
     * @param[in]  condition  The condition
     * @param[in]  fn         The function
     */
    void enqueue(thread_switchable&                                   pivot,
                 const std::function<bool(fb::thread&)>&              condition,
                 const std::function<async::task<void>(fb::thread&)>& fn);

    /**
     * @brief      { function_description }
     *
     * @param      pivot       The pivot
     * @param[in]  fn          The function
     *
     * @tparam     ReturnType  { description }
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
     * @brief      { function_description }
     *
     * @param      pivot  The pivot
     * @param[in]  fn     The function
     */
    void enqueue(thread_switchable& pivot, const std::function<async::task<void>(fb::thread&)>& fn);

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
     * @brief      { function_description }
     *
     * @param      pivot      The pivot
     * @param[in]  condition  The condition
     * @param[in]  fn         The function
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                                   pivot,
                                             const std::function<bool(fb::thread&)>&              condition,
                                             const std::function<async::task<void>(fb::thread&)>& fn);
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
     * @brief      { function_description }
     *
     * @param      pivot  The pivot
     * @param[in]  fn     The function
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> dispatch(thread_switchable&                                   pivot,
                                             const std::function<async::task<void>(fb::thread&)>& fn);

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
    void settimer(const fb::timer::handle_callback_type& fn, const fb::model::timespan& duration);
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

#endif