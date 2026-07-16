#ifndef __THREAD_H__
#define __THREAD_H__

#include <thread>
#include <functional>
#include <future>
#include <atomic>
#include <queue>
#include <stdexcept>
#include <type_traits>
#include <fb/execution_context.h>
#include <fb/logger.h>
#include <fb/timer.h>
#include <async/propagation.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_then.h>
#include <fb/model/datetime.h>
#include <unordered_set>
#include <fb/lua.h>
#include <fb/synchronized.h>
#include <boost/stacktrace.hpp>
#include <memory>

namespace fb {

class thread_container;

class thread : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

    friend class thread_container;

    struct retry_exception : std::exception
    {
        const char* what() const noexcept override
        {
            return "retry requested";
        }
    };

    template <typename T>
    class builder;

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
    std::unordered_set<const void*>                   _ptrs;
    timer_list                                        _timers;
    std::unique_ptr<void, std::function<void(void*)>> _data;

private:
    fb::synchronized<std::queue<std::function<void()>>> _queue;

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

    template <typename T, typename OnSuccess, typename OnFailure>
    void run_retry_loop(handle_func_type<T>&&     fn,
                        size_t                    max_retries,
                        async::propagation::token context,
                        OnSuccess&&               on_success,
                        OnFailure&&               on_failure)
    {
        const auto     retry_limit = max_retries;
        auto           attempts    = std::make_shared<size_t>(0);
        auto           fn_holder   = std::make_shared<handle_func_type<T>>(std::move(fn));
        constexpr auto retry_delay = 100ms;

        auto retry_func = std::make_shared<std::function<void()>>();
        *retry_func     = [=,
                       this,
                       on_success = std::forward<OnSuccess>(on_success),
                       on_failure = std::forward<OnFailure>(on_failure)]() mutable {
            execution_context::pending(context);
            async::awaitable_then(
                (*fn_holder)(*this),
                [=, this, on_success = std::move(on_success), on_failure = std::move(on_failure)](
                    async::awaitable_result<T> result) mutable {
                    try
                    {
                        if constexpr (std::is_same_v<T, void>)
                        {
                            result();
                            on_success();
                        }
                        else
                        {
                            on_success(result());
                        }
                    }
                    catch (const retry_exception&)
                    {
                        if (*attempts < retry_limit)
                        {
                            (*attempts)++;
                            async::awaitable_then(
                                this->sleep(retry_delay),
                                [=, this, on_success = std::move(on_success), on_failure = std::move(on_failure)](
                                    async::awaitable_result<void> sleep_result) mutable {
                                    try
                                    {
                                        sleep_result();
                                        {
                                            auto guard = this->_queue.enter_write();
                                            guard.value().push(*retry_func);
                                        }
                                    }
                                    catch (std::exception& e)
                                    {
                                        on_failure(e);
                                    }
                                    catch (...)
                                    {
                                        auto unknown = std::runtime_error("unknown error");
                                        on_failure(unknown);
                                    }
                                });
                        }
                        else
                        {
                            auto exhausted = std::runtime_error("max retries exceeded");
                            on_failure(exhausted);
                        }
                    }
                    catch (std::exception& e)
                    {
                        on_failure(e);
                    }
                    catch (...)
                    {
                        auto unknown = std::runtime_error("unknown error");
                        on_failure(unknown);
                    }
                });
        };

        {
            auto guard = this->_queue.enter_write();
            guard.value().push(*retry_func);
        }
    }

    template <typename T>
    async::task<T> dispatch_with_retry(handle_func_type<T>&&     fn,
                                       size_t                    max_retries,
                                       async::propagation::token context = {})
    {
        auto promise = std::make_shared<async::task_completion_source<T>>();

        if constexpr (std::is_same_v<T, void>)
        {
            this->run_retry_loop<T>(
                std::move(fn),
                max_retries,
                std::move(context),
                [promise]() {
                    promise->set_value();
                },
                [promise](std::exception& e) {
                    promise->set_exception(std::make_exception_ptr(e));
                });
        }
        else
        {
            this->run_retry_loop<T>(
                std::move(fn),
                max_retries,
                std::move(context),
                [promise](T&& value) {
                    promise->set_value(std::move(value));
                },
                [promise](std::exception& e) {
                    promise->set_exception(std::make_exception_ptr(e));
                });
        }

        return promise->task();
    }

    template <typename T>
    void enqueue_with_retry(handle_func_type<T>&&     fn,
                            size_t                    max_retries,
                            handle_error_type         error,
                            std::function<void()>     on_complete,
                            async::propagation::token context = {})
    {
        if constexpr (std::is_same_v<T, void>)
        {
            this->run_retry_loop<T>(
                std::move(fn),
                max_retries,
                std::move(context),
                [on_complete = std::move(on_complete)]() {
                    if (on_complete)
                        on_complete();
                },
                std::move(error));
        }
        else
        {
            this->run_retry_loop<T>(
                std::move(fn),
                max_retries,
                std::move(context),
                [on_complete = std::move(on_complete)](T&&) {
                    if (on_complete)
                        on_complete();
                },
                std::move(error));
        }
    }

public:
    static handle_error_type on_error(handle_error_type handler)
    {
        if (handler)
            return handler;

        return [](std::exception&) {
        };
    }

    std::thread::id            id() const;
    uint8_t                    index() const;
    void                       join();
    void                       exit();
    template <typename T> void data(std::unique_ptr<T> value)
    {
        this->assert_exec();
        auto deleter = [](void* ptr) {
            delete static_cast<T*>(ptr);
        };
        auto* raw   = value.release();
        this->_data = std::unique_ptr<void, std::function<void(void*)>>(raw, deleter);
    }

    template <typename ReturnType> ReturnType* data() const
    {
        this->assert_exec();
        if (this->_data == nullptr)
            return nullptr;
        return static_cast<ReturnType*>(this->_data.get());
    }

public:
    std::shared_ptr<fb::timer>      settimer(fb::timer::handle_callback_type&& fn,
                                             const fb::model::timespan&        duration,
                                             fb::timer::repeat_type            repeat = fb::timer::repeat_type::repeat);
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration);

    template <typename T = void> [[nodiscard]] builder<T> new_builder()
    {
        return builder<T>(*this);
    }

    void enqueue(handle_func_type<void>&&  fn,
                 handle_error_type&&       error,
                 std::function<void()>&&   callback,
                 async::propagation::token context = {});

    template <typename ReturnType> void enqueue(handle_func_type<ReturnType>&&      fn,
                                                handle_error_type&&                 error,
                                                std::function<void(ReturnType&&)>&& callback,
                                                async::propagation::token           context = {})
    {
        auto  fn_holder = std::make_shared<handle_func_type<ReturnType>>(std::move(fn));
        auto  guard     = this->_queue.enter_write();
        auto& queue     = guard.value();
        queue.push([fn_holder = std::move(fn_holder),
                    error     = std::move(error),
                    callback  = std::move(callback),
                    context   = std::move(context),
                    this]() {
            execution_context::pending(context);
            async::awaitable_then((*fn_holder)(*this),
                                  [fn_holder, error = std::move(error), callback = std::move(callback)](
                                      async::awaitable_result<ReturnType> result) {
                                      try
                                      {
                                          if (callback)
                                              callback(result());
                                          else
                                              result();
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

    template <typename ReturnType> async::task<ReturnType> dispatch(handle_func_type<ReturnType>&& fn,
                                                                    async::propagation::token      context = {})
    {
        auto promise = std::make_shared<async::task_completion_source<ReturnType>>();
        this->enqueue<ReturnType>(
            std::move(fn),
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
            },
            [promise](ReturnType&& value) {
                promise->set_value(std::move(value));
            },
            std::move(context));
        return promise->task();
    }

    [[nodiscard]] async::task<void> dispatch(handle_func_type<void>&& fn, async::propagation::token context = {});
    [[nodiscard]] async::task<void> switching();
    size_t                          queue_size() const;
    std::string                     to_string() const;
};

template <typename T>
class thread::builder
{
    friend class thread;

private:
    thread& _thread;

public:
    handle_func_type<T>       func;
    handle_error_type         on_error;
    std::function<void()>     on_complete;
    size_t                    retry_count = 0;
    async::propagation::token context;

    void enqueue()
    {
        if (this->func == nullptr)
            throw std::runtime_error("thread builder: func is not set");

        auto resolved = execution_context::token(this->context);
        auto error    = thread::on_error(std::move(this->on_error));
        auto complete = this->on_complete ? std::move(this->on_complete) : std::function<void()>{};

        if (this->retry_count > 0)
        {
            this->_thread.enqueue_with_retry<T>(std::move(this->func),
                                                this->retry_count,
                                                error,
                                                std::move(complete),
                                                std::move(resolved));
        }
        else if constexpr (std::is_same_v<T, void>)
        {
            this->_thread.enqueue(std::move(this->func), std::move(error), std::move(complete), std::move(resolved));
        }
        else
        {
            this->_thread.enqueue<T>(
                std::move(this->func),
                std::move(error),
                [complete = std::move(complete)](T&&) {
                    if (complete)
                        complete();
                },
                std::move(resolved));
        }
    }

    [[nodiscard]] async::task<T> dispatch()
    {
        if (this->func == nullptr)
            throw std::runtime_error("thread builder: func is not set");

        auto resolved = execution_context::token(this->context);

        if (this->retry_count > 0)
        {
            return this->_thread.dispatch_with_retry<T>(std::move(this->func), this->retry_count, std::move(resolved));
        }

        if constexpr (std::is_same_v<T, void>)
            return this->_thread.dispatch(std::move(this->func), std::move(resolved));
        else
            return this->_thread.dispatch<T>(std::move(this->func), std::move(resolved));
    }

    explicit builder(thread& owner) :
        _thread(owner)
    { }
};

} // namespace fb

#endif // !__THREAD_H__
