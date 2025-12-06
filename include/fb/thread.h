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
#include <fb/locker.h>
#include <boost/stacktrace.hpp>

namespace fb {

class thread_container;

class thread : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

    friend class thread_container;

public:
    template <typename ReturnType>
    using handle_func_type  = std::function<async::task<ReturnType>(fb::thread&)>;
    using handle_error_type = std::function<void(std::exception&)>;
    using timer_list        = std::vector<std::shared_ptr<timer>>;

private:
    uint8_t           _index = 0;
    std::atomic<bool> _exit  = false;
    std::thread       _thread;

private:
    std::unordered_set<const void*> _ptrs;
    timer_list                      _timers;
    void*                           _data = nullptr;

private:
    fb::locker<std::queue<std::function<void()>>> _queue;

public:
    thread(uint8_t index);
    ~thread();
    thread(thread&&)                  = delete;
    thread& operator= (thread&)       = delete;
    thread& operator= (const thread&) = delete;

private:
    void on_thread(uint8_t index);
    void on_idle();
    void assert_exec() const;

public:
    std::thread::id            id() const;
    uint8_t                    index() const;
    void                       join();
    void                       exit();
    template <typename T> void data(T* value)
    {
        this->assert_exec();
        this->_data = static_cast<void*>(value);
    }

    template <typename ReturnType> ReturnType* data() const
    {
        this->assert_exec();
        return static_cast<ReturnType*>(this->_data);
    }

public:
    std::shared_ptr<fb::timer>      settimer(const fb::timer::handle_callback_type& fn,
                                             const fb::model::timespan&             duration,
                                             fb::timer::repeat_type                 repeat = fb::timer::repeat_type::repeat);
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration);
    void                            enqueue(const handle_func_type<void>& fn, const handle_error_type& error, const std::function<void()>& callback);

    template <typename ReturnType> void enqueue(const handle_func_type<ReturnType>& fn, const handle_error_type& error, const std::function<void(ReturnType&&)>& callback)
    {
        this->_queue.write([=, this](auto& queue) {
            queue.push([=, this]() {
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
        });
    }

    template <typename ReturnType> async::task<ReturnType> dispatch(const handle_func_type<ReturnType>& fn)
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

    [[nodiscard]] async::task<void> dispatch(const handle_func_type<void>& fn);
    [[nodiscard]] async::task<void> switching();
    size_t                          queue_size() const;
    std::string                     to_string() const;

    template <typename RetryFunc>
    async::task<bool> enqueue_with_retry(
        RetryFunc&&              fn,
        size_t                   max_retries = 10,
        const handle_error_type& error       = [](std::exception& e) {
        })
    {
        static_assert(std::is_same_v<decltype(fn(*this)), async::task<bool>>, "RetryFunc must return async::task<bool>");

        auto           promise     = std::make_shared<async::task_completion_source<bool>>();
        auto           retry_count = std::make_shared<size_t>(0);
        constexpr auto retry_delay = 100ms;

        auto retry_func = std::make_shared<std::function<void()>>();
        *retry_func     = [=, this]() mutable {
            async::awaitable_then(fn(*this), [=, this](async::awaitable_result<bool> result) mutable {
                try
                {
                    auto success = result();
                    if (success)
                    {
                        promise->set_value(true);
                    }
                    else if (*retry_count < max_retries)
                    {
                        (*retry_count)++;
                        // Sleep before retry
                        async::awaitable_then(this->sleep(retry_delay), [=, this](async::awaitable_result<void> sleep_result) {
                            try
                            {
                                sleep_result();
                                // Re-enqueue after sleep
                                this->_queue.write([retry_func](auto& queue) {
                                    queue.push(*retry_func);
                                });
                            }
                            catch (std::exception& e)
                            {
                                error(e);
                                promise->set_exception(std::make_exception_ptr(e));
                            }
                            catch (...)
                            {
                                promise->set_exception(std::make_exception_ptr(std::runtime_error("unknown error")));
                            }
                        });
                    }
                    else
                    {
                        promise->set_value(false); // Max retries exceeded
                    }
                }
                catch (std::exception& e)
                {
                    error(e);
                    promise->set_exception(std::make_exception_ptr(e));
                }
                catch (...)
                {
                    promise->set_exception(std::make_exception_ptr(std::runtime_error("unknown error")));
                }
            });
        };

        // First attempt
        this->_queue.write([retry_func](auto& queue) {
            queue.push(*retry_func);
        });

        return promise->task();
    }
};

} // namespace fb

#endif // !__THREAD_H__
