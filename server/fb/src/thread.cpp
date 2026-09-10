#include <fb/execution_context.h>
#include <fb/logger.h>
#include <fb/thread.h>
#include <sstream>

fb::thread::thread(uint8_t index) :
    _index(index)
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

    while (!this->_exit)
    {
        std::function<void()> func;

        {
            auto  guard = this->_queue.enter_write();
            auto& queue = guard.value();
            if (queue.empty() == false)
            {
                func = queue.front();
                queue.pop();
            }
        }

        if (func != nullptr)
        {
            func();
        }
        else
        {
            auto begin = fb::model::datetime();
            this->on_idle();
            auto elapsed = fb::model::datetime() - begin;

            if (elapsed < term)
                std::this_thread::sleep_for(std::chrono::milliseconds((term - elapsed).total_milliseconds()));
        }
    }
}

void fb::thread::on_idle()
{
    auto now = fb::model::datetime();
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
    this->_thread.join();
}

void fb::thread::exit()
{
    if (this->_exit)
        return;

    this->_exit = true;
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
                co_await callback(fb::model::datetime(), this->_thread.get_id());
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
    auto  fn_holder = std::make_shared<handle_func_type<void>>(std::move(fn));
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
        this->enqueue(
            std::move(*fn_holder),
            [promise](std::exception& e) {
                promise->set_exception(std::make_exception_ptr(e));
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