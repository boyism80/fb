#include <fb/thread.h>

fb::thread::thread(uint8_t index) :
    _index(index)
{
    this->_thread = std::thread(std::bind(&fb::thread::handle_thread, this, std::placeholders::_1), index);
}

fb::thread::~thread()
{
    this->exit();
}

void fb::thread::handle_thread(uint8_t index)
{
    constexpr auto term = 100ms;

    while (!this->_exit)
    {
        std::function<void()> func;
        {
            auto _ = std::lock_guard(this->_mutex_queue);

            if (this->_queue.empty() == false)
            {
                func = this->_queue.front();
                this->_queue.pop();
            }
        }

        if (func != nullptr)
        {
            func();
        }
        else
        {
            auto begin = fb::model::datetime();
            this->handle_idle();
            auto elapsed = fb::model::datetime() - begin;

            if (elapsed < term)
                std::this_thread::sleep_for(std::chrono::milliseconds((term - elapsed).total_milliseconds()));
        }
    }
}

void fb::thread::handle_idle()
{
    this->_timers.write([&](auto& timers) {
        auto now = fb::model::datetime();
        for (int i = timers.size() - 1; i >= 0; i--)
        {
            auto timer = timers[i].get();
            if (timer->canceled())
            {
                timers.erase(timers.begin() + i);
                continue;
            }

            if (now < timer->begin + timer->duration)
                continue;

            auto repeat = timer->repeat;
            auto fn     = fb::timer::handle_callback_type{timer->fn};
            async::awaitable_then(fn(now, this->_thread.get_id()), [timer, repeat, now](auto result) {
                if (repeat == fb::timer::repeat_type::once)
                    timer->begin = now;
            });

            if (timer->repeat == fb::timer::repeat_type::once)
            {
                timers.erase(timers.begin() + i);
            }
        }
    });
}

void fb::thread::assert_exec() const
{
    if (std::this_thread::get_id() != this->id())
        throw std::runtime_error("cannot push pointer value. thread mismatched.");
}

void fb::thread::exit()
{
    if (this->_exit)
        return;

    this->_exit = true;
    try
    {
        if (this->_thread.joinable())
            this->_thread.join();
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    this->_timers.write([](auto& timers) {
        timers.clear();
    });
}

std::shared_ptr<fb::timer> fb::thread::settimer(const fb::timer::handle_callback_type& fn,
                                                const fb::model::timespan&             duration,
                                                fb::timer::repeat_type                 repeat)
{
    return this->_timers.write([&](auto& timers) {
        auto ptr = new fb::timer(
            [this, fn](const fb::model::datetime&, std::thread::id) -> async::task<void> {
                auto index = this->_index;
                try
                {
                    co_await fn(fb::model::datetime(), this->_thread.get_id());
                }
                catch (std::exception& e)
                {
                    fb::logger::fatal(std::format("timer error in thread {} : {}", index, e.what()));
                }
                co_return;
            },
            duration,
            repeat);

        auto shared_ptr = std::shared_ptr<fb::timer>(ptr);
        timers.push_back(shared_ptr);
        return shared_ptr;
    });
}

async::task<void> fb::thread::sleep(const fb::model::timespan& delay)
{
    auto promise = std::make_shared<async::task_completion_source<void>>();
    this->settimer(
        [promise](auto& datetime, auto thread_id) -> async::task<void> {
            promise->set_value();
            co_return;
        },
        delay,
        fb::timer::repeat_type::once);

    return promise->task();
}

void fb::thread::enqueue(const handle_func_type<void>& fn,
                         const handle_error_type&      error,
                         const std::function<void()>&  callback)
{
    auto _ = std::lock_guard(_mutex_queue);

    this->_queue.push([=, this]() {
        async::awaitable_then(fn(*this), [=](async::awaitable_result<void> result) {
            try
            {
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

async::task<void> fb::thread::dispatch(const handle_func_type<void>& fn)
{
    auto promise = std::make_shared<async::task_completion_source<void>>();
    this->enqueue(
        fn,
        [promise](std::exception& e) {
            promise->set_exception(std::make_exception_ptr(e));
        },
        [promise]() {
            promise->set_value();
        });
    return promise->task();
}

async::task<void> fb::thread::switching()
{
    if (this->id() == std::this_thread::get_id())
        co_return;

    co_await this->dispatch([](auto& thread) -> async::task<void> {
        co_return;
    });
}

std::thread::id fb::thread::id() const
{
    return this->_thread.get_id();
}

uint8_t fb::thread::index() const
{
    return this->_index;
}