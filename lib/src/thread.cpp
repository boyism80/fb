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
    this->_data = value;
}

void* fb::thread::data() const
{
    return _data;
}

void fb::thread::settimer(const fb::timer_callback& fn, const fb::model::timespan& duration, bool disposable)
{
    auto _ = std::lock_guard(this->_mutex_timer);

    auto timer = new fb::timer(
        [this, fn](const fb::model::datetime&, std::thread::id) -> async::task<void> {
            fn(fb::model::datetime(), this->_thread.get_id());
            co_return;
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

std::thread::id fb::thread::id() const
{
    return this->_thread.get_id();
}

uint8_t fb::thread::index() const
{
    return this->_index;
}

fb::threads::threads(boost::asio::io_context& context) :
    _context(context)
{
    auto count = fb::config::get()["thread"]["logic"].asUInt();
    if (count > 0)
        this->_keys = std::make_unique<std::thread::id[]>(count);

    for (int i = 0; i < count; i++)
    {
        auto ptr       = std::make_unique<fb::thread>(i);
        auto id        = ptr->id();
        this->_keys[i] = id;
        this->_threads.insert({id, std::move(ptr)});
    }
}

fb::thread* fb::threads::at(uint8_t index) const
{
    if (this->_threads.size() == 0)
        return nullptr;

    if (index > this->_threads.size() - 1)
        return nullptr;

    auto& id = this->_keys[index];
    return this->at(id);
}

fb::thread* fb::threads::at(std::thread::id id) const
{
    auto found = this->_threads.find(id);
    if (found == this->_threads.end())
        return nullptr;
    else
        return found->second.get();
}

fb::thread* fb::threads::modular(uint32_t id) const
{
    if (this->_threads.size() == 0)
        return nullptr;

    auto index = id % this->size();
    return this->at(index);
}

fb::thread* fb::threads::current()
{
    auto id    = std::this_thread::get_id();
    auto found = this->_threads.find(id);
    if (found == this->_threads.end())
        return nullptr;
    else
        return found->second.get();
}

const fb::thread* fb::threads::current() const
{
    auto id    = std::this_thread::get_id();
    auto found = this->_threads.find(id);
    if (found == this->_threads.end())
        return nullptr;
    else
        return found->second.get();
}

uint8_t fb::threads::count() const
{
    return (uint8_t)this->_threads.size();
}

bool fb::threads::empty() const
{
    return this->_threads.size() == 0;
}

bool fb::threads::valid(uint8_t index) const
{
    auto current = this->current();
    return current->index() == index;
}

bool fb::threads::valid(fb::thread* thread) const
{
    if (thread == nullptr)
        return this->empty();

    return this->valid(*thread);
}

bool fb::threads::valid(fb::thread& thread) const
{
    auto current = this->current();
    return current != nullptr && current->id() == thread.id();
}

size_t fb::threads::size() const
{
    return this->_threads.size();
}

void fb::threads::settimer(const fb::timer_callback& fn, const fb::model::timespan& duration)
{
    if (this->_threads.empty())
    {
        throw std::runtime_error("cannot set timer. logic thread does not exists");
    }
    else
    {
        for (auto& [key, value] : this->_threads)
        {
            value->settimer(fn, duration);
        }
    }
}

void fb::threads::exit()
{

    for (auto& thread : this->_threads)
    {
        thread.second->exit();
    }

    this->_threads.clear();
}

fb::thread* fb::threads::operator[] (uint8_t index) const
{
    return this->at(index);
}

fb::thread* fb::threads::operator[] (std::thread::id id) const
{
    return this->at(id);
}