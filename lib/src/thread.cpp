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
    auto _ = std::lock_guard(this->_mutex_timer);

    auto indices = std::vector<uint32_t>();
    auto now     = fb::model::datetime();
    for (int i = this->_timers.size() - 1; i >= 0; i--)
    {
        auto& timer   = this->_timers[i];
        auto  elapsed = fb::model::datetime() - timer->begin;
        if (timer->duration > elapsed)
            continue;

        try
        {
            timer->fn(now, this->_thread.get_id());
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
        if (timer->disposable)
            this->_timers.erase(this->_timers.begin() + i);
    }
}

void fb::thread::assert_exec() const
{
    if(std::this_thread::get_id() != this->id())
        throw std::runtime_error("cannot push pointer value. thread mismatched.");
}

void fb::thread::exit()
{
    if (this->_exit)
        return;

    this->_exit = true;
    if (this->_thread.joinable())
        this->_thread.join();

    this->_mutex_timer.lock();
    this->_timers.clear();
    this->_mutex_timer.unlock();
}

void fb::thread::data(void* value)
{
    this->assert_exec();
    this->_data = value;
}

void* fb::thread::data() const
{
    this->assert_exec();
    return _data;
}

void fb::thread::push_ptr(void* ptr)
{
    this->assert_exec();
    this->_ptrs.insert(static_cast<void*>(ptr));
}

void fb::thread::pop_ptr(void* ptr)
{
    this->assert_exec();
    this->_ptrs.erase(static_cast<void*>(ptr));
}

void fb::thread::settimer(const fb::timer::handle_callback_type& fn,
                          const fb::model::timespan&             duration,
                          bool                                   disposable)
{
    auto _ = std::lock_guard(this->_mutex_timer);

    auto timer = new fb::timer(
        [this, fn](const fb::model::datetime&, std::thread::id) -> async::task<void> {
            co_await fn(fb::model::datetime(), this->_thread.get_id());
        },
        duration,
        disposable);
    this->_timers.push_back(std::unique_ptr<fb::timer>(timer));
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
        true);

    return promise->task();
}

void fb::thread::enqueue(const std::function<async::task<void>()>&   fn,
                         const std::function<void(std::exception&)>& error,
                         const std::function<void()>&                callback)
{
    auto _ = std::lock_guard(_mutex_queue);

    this->_queue.push([=]() {
        async::awaitable_then(fn(), [=](async::awaitable_result<void> result) {
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

async::task<void> fb::thread::dispatch(const std::function<async::task<void>()>& fn)
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
    return this->dispatch([]() -> async::task<void> {
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