#include <fb/thread.h>

#include <fb/execution_context.h>
#include <fb/logger.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

fb::thread::thread(uint8_t index) :
    _index(index),
    _scheduler([this](std::function<void()> fn) {
        this->push(std::move(fn));
    })
{
    this->_thread = std::thread(std::bind(&fb::thread::on_thread, this, std::placeholders::_1), index);
}

fb::thread::~thread()
{
    this->exit();
}

void fb::thread::on_thread(uint8_t index)
{
    constexpr auto term = 100ms;

    async_shared_mutex::thread_scheduler = &this->_scheduler;

    auto next_idle = fb::model::datetime::utc_now() + term;
    while (this->_exit == false)
    {
        std::function<void()> func;

        {
            std::unique_lock<std::mutex> wake(this->_wake_mutex);
            auto                         ready = [this]() {
                auto guard = this->_queue.enter_read();
                return guard.value().empty() == false || this->_exit.load();
            };

            auto now = fb::model::datetime::utc_now();
            if (ready() == false && now < next_idle)
            {
                auto remaining = next_idle - now;
                this->_wake.wait_for(wake, std::chrono::milliseconds(remaining.total_milliseconds()), ready);
            }

            if (this->_exit)
                break;

            auto  guard = this->_queue.enter_write();
            auto& queue = guard.value();
            if (queue.empty() == false)
            {
                func = std::move(queue.front());
                queue.pop();
            }
        }

        auto now = fb::model::datetime::utc_now();
        if (func != nullptr && now < next_idle)
        {
            func();
        }
        else if (func != nullptr)
        {
            this->on_idle();
            next_idle = fb::model::datetime::utc_now() + term;
            func();
        }
        else
        {
            this->on_idle();
            next_idle  += term;
            auto after  = fb::model::datetime::utc_now();
            if (next_idle <= after)
                next_idle = after + term;
        }
    }
}

void fb::thread::push(std::function<void()> fn)
{
    std::lock_guard<std::mutex> wake(this->_wake_mutex);
    {
        auto guard = this->_queue.enter_write();
        guard.value().push(std::move(fn));
    }
    this->_wake.notify_one();
}

void fb::thread::on_idle()
{
    auto now = fb::model::datetime::utc_now();
    for (int i = static_cast<int>(this->_timers.size()) - 1; i >= 0; i--)
    {
        auto timer = this->_timers[i];
        if (timer->canceled())
        {
            this->_timers.erase(this->_timers.begin() + i);
            continue;
        }

        if (timer->running())
            continue;

        if (now < timer->begin + timer->duration)
            continue;

        if (timer->try_begin_run() == false)
            continue;

        if (timer->repeat == fb::timer::repeat_type::repeat)
            timer->begin = now;

        auto index = this->_index;
        execution_context::pending(timer->context);

        // Invoke timer->fn in place and keep this shared_ptr until the task
        // finishes. Copying the std::function and dropping it at the end of this
        // iteration frees coroutine-lambda captures while lua/script_timer is
        // still suspended (ASan heap-use-after-free).
        async::awaitable_then(timer->fn(now, this->_thread.get_id()), [timer, index](auto result) {
            try
            {
                result();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("timer error in thread {} : {}", index, e.what());
            }
            catch (...)
            {
                fb::logger::fatal("timer error in thread {} : unknown exception", index);
            }
            timer->end_run();
        });

        if (timer->repeat == fb::timer::repeat_type::once)
        {
            this->_timers.erase(this->_timers.begin() + i);
        }
    }
}

void fb::thread::assert_exec() const
{
    if (std::this_thread::get_id() != this->id())
        throw std::runtime_error("cannot push pointer value. thread mismatched.");
}

void fb::thread::join()
{
    if (this->_thread.joinable() == false)
        return;

    // Joining itself throws, and detaching would let on_thread() keep reading this destroyed object.
    // acceptor::run() joins logic threads on every exit path, so reaching this is a bug; crash loudly.
    if (this->_thread.get_id() == std::this_thread::get_id())
    {
        fb::logger::fatal("thread {}: join requested from the thread itself; aborting", this->_index);
        std::abort();
    }

    this->_thread.join();
}

void fb::thread::exit()
{
    std::lock_guard<std::mutex> wake(this->_wake_mutex);
    if (this->_exit)
        return;

    this->_exit = true;
    this->_wake.notify_one();
}

std::shared_ptr<fb::timer> fb::thread::settimer(fb::timer::handle_callback_type&& fn,
                                                const fb::model::timespan&        duration,
                                                fb::timer::repeat_type            repeat)
{
    if (this->id() != std::this_thread::get_id())
    {
        throw std::runtime_error(std::format("cannot set timer. thread mismatched. stacktrace : {}",
                                             boost::stacktrace::to_string(boost::stacktrace::stacktrace())));
    }

    auto snapshot = execution_context::token();
    auto ptr      = new fb::timer(
        [this, fn](const fb::model::datetime&, std::thread::id) -> async::task<void> {
            auto index    = this->_index;
            auto callback = fn;
            try
            {
                co_await callback(fb::model::datetime::utc_now(), this->_thread.get_id());
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("timer error in thread {} : {}", index, e.what());
            }
            catch (...)
            {
                fb::logger::fatal("timer error in thread {} : unknown exception", index);
            }
            co_return;
        },
        duration,
        repeat,
        std::move(snapshot));

    auto shared_ptr = std::shared_ptr<fb::timer>(ptr);
    this->_timers.push_back(shared_ptr);
    return shared_ptr;
}

async::task<void> fb::thread::sleep(const fb::model::timespan& delay)
{
    if (delay.total_milliseconds() == 0)
        co_return;

    auto promise = std::make_shared<async::task_completion_source<void>>();
    this->settimer(
        [promise](auto& datetime, auto thread_id) -> async::task<void> {
            promise->set_value();
            co_return;
        },
        delay,
        fb::timer::repeat_type::once);

    co_return co_await promise->task();
}

void fb::thread::enqueue(handle_func_type<void>&&  fn,
                         handle_error_type&&       error,
                         std::function<void()>&&   callback,
                         async::propagation::token context)
{
    auto fn_holder = std::make_shared<handle_func_type<void>>(std::move(fn));
    this->push([fn_holder = std::move(fn_holder),
                error     = std::move(error),
                callback  = std::move(callback),
                context   = std::move(context),
                this]() {
        execution_context::pending(context);
        async::awaitable_then((*fn_holder)(*this),
                              [fn_holder, error = std::move(error), callback = std::move(callback)](
                                  async::awaitable_result<void> result) {
                                  try
                                  {
                                      result();
                                      if (callback)
                                          callback();
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

async::task<void> fb::thread::dispatch(handle_func_type<void>&& fn, async::propagation::token context)
{
    auto promise   = std::make_shared<async::task_completion_source<void>>();
    auto fn_holder = std::make_shared<handle_func_type<void>>(std::move(fn));
    if (this->id() == std::this_thread::get_id())
    {
        execution_context::pending(context);
        async::awaitable_then((*fn_holder)(*this), [fn_holder, promise](auto result) {
            try
            {
                result();
                promise->set_value();
            }
            catch (std::exception& e)
            {
                promise->set_exception(std::current_exception());
            }
            catch (...)
            {
                promise->set_exception(std::make_exception_ptr(std::runtime_error("unknown error")));
            }
        });
    }
    else
    {
        this->enqueue(
            std::move(*fn_holder),
            [promise](std::exception& e) {
                promise->set_exception(std::current_exception());
            },
            [promise]() {
                promise->set_value();
            },
            std::move(context));
    }

    return promise->task();
}

async::task<void> fb::thread::switching()
{
    co_await this->dispatch(
        [](auto& thread) -> async::task<void> {
            co_return;
        },
        execution_context::token());
}

std::thread::id fb::thread::id() const
{
    return this->_thread.get_id();
}

uint8_t fb::thread::index() const
{
    return this->_index;
}

size_t fb::thread::queue_size() const
{
    auto guard = this->_queue.enter_read();
    return guard.value().size();
}

std::string fb::thread::to_string() const
{
    std::ostringstream oss;
    oss << "thread[" << static_cast<int>(this->_index) << ":" << this->_thread.get_id() << "]";
    return oss.str();
}