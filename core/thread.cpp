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

    while(!this->_exit)
    {
        auto completed = false;
        completed = this->_queue.dequeue([this](auto&& x) mutable
        {
            async::awaitable_then(x.func(), [callback = x.callback](async::awaitable_result<void> result) 
            {
                if (callback != nullptr)
                    callback();
            });
        });
        if(completed)
            continue;

        auto begin = datetime();
        this->handle_idle();
        auto elapsed = datetime() - begin;

        if (elapsed < term)
            std::this_thread::sleep_for(std::chrono::milliseconds((term - elapsed).total_milliseconds()));
    }
}

void fb::thread::handle_idle()
{
    auto _ = std::lock_guard(this->_mutex_timer);

    auto indices = std::vector<uint32_t>();
    auto now = datetime();
    for(int i = this->_timers.size() - 1; i >= 0; i--)
    {
        auto& timer = this->_timers[i];
        auto elapsed = datetime() - timer->begin;
        if (timer->duration > elapsed)
            continue;

        timer->fn(now, this->_thread.get_id());
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

async::task<void> fb::thread::dispatch(const std::function<async::task<void>()>& fn, const timespan& delay, uint32_t priority)
{
    auto promise = std::make_shared<async::task_completion_source<void>>();
    if (delay > 0s)
    {
        this->settimer([this, priority, promise, fn](auto time, auto id) mutable
        {
            this->_queue.enqueue(fb::thread::task(fn, [promise]() 
            {
                promise->set_value();
            }), priority);
        }, delay, true);
    }
    else
    {
        this->_queue.enqueue(fb::thread::task(fn, [promise]() 
        {
            promise->set_value();
        }), priority);
    }

    return promise->task();
}

void fb::thread::post(const std::function<async::task<void>()>& fn, const timespan& delay, uint32_t priority)
{
    if (delay > 0s)
    {
        this->settimer([this, priority, fn](auto time, auto id) mutable
        {
            this->_queue.enqueue(fb::thread::task(fn, nullptr), priority);
        }, delay, true);
    }
    else
    {
        this->_queue.enqueue(fb::thread::task(fn, nullptr), priority);
    }
}

async::task<void> fb::thread::dispatch(uint32_t priority)
{
    co_await this->dispatch([] () -> async::task<void>
    {
        co_return;
    }, 0s, priority);
}

void fb::thread::settimer(const fb::timer_callback& fn, const timespan& duration, bool disposable)
{
    auto _ = std::lock_guard(this->_mutex_timer);

    auto timer = new fb::timer([this, fn](const datetime&, std::thread::id)
    {
        fn(datetime(), this->_thread.get_id());
    }, duration, disposable);
    this->_timers.push_back(std::unique_ptr<fb::timer>(timer));
}

async::task<void> fb::thread::sleep(const timespan& delay)
{
    co_await this->dispatch([] () -> async::task<void>
    {
        co_return;
    }, delay);
}

fb::thread::task::task(const fb::thread::task::func_type& func) : func(func)
{}

fb::thread::task::task(const fb::thread::task::func_type& func, const fb::thread::task::callback_type& callback) : func(func), callback(callback)
{}

fb::thread::task::task(fb::thread::task&& r) noexcept : func(std::move(r.func)), callback(std::move(r.callback))
{
    
}

fb::thread::task::~task()
{
}

void fb::thread::task::operator = (fb::thread::task&& r) noexcept
{
    this->func = std::move(r.func);
    this->callback = std::move(r.callback);
}

std::thread::id fb::thread::id() const
{
    return this->_thread.get_id();
}

uint8_t fb::thread::index() const
{
    return this->_index;
}

fb::threads::threads(boost::asio::io_context& context, uint8_t count) : 
    _context(context)
{
    if(count > 0)
        this->_keys = std::make_unique<std::thread::id[]>(count);

    for(int i = 0; i < count; i++)
    {
        auto ptr = std::make_unique<fb::thread>(i);
        auto id = ptr->id();
        this->_keys[i] = id;
        this->_threads.insert(std::make_pair(id, std::move(ptr)));
    }
}

fb::thread* fb::threads::at(uint8_t index) const
{
    if (this->_threads.size() == 0)
        return nullptr;

    if(index > this->_threads.size() - 1)
        return nullptr;

    auto& id = this->_keys[index];
    return this->at(id);
}

fb::thread* fb::threads::at(std::thread::id id) const
{
    auto found = this->_threads.find(id);
    if(found == this->_threads.end())
        return nullptr;
    else
        return found->second.get();
}

fb::thread* fb::threads::current()
{
    auto id = std::this_thread::get_id();
    auto found = this->_threads.find(id);
    if(found == this->_threads.end())
        return nullptr;
    else
        return found->second.get();
}

const fb::thread* fb::threads::current() const
{
    auto id = std::this_thread::get_id();
    auto found = this->_threads.find(id);
    if(found == this->_threads.end())
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
    if(thread == nullptr)
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

async::task<void> fb::threads::dispatch(const std::function<async::task<void>()>& fn, const timespan& delay)
{
    auto current = this->current();
    if (current == nullptr)
        throw std::runtime_error("therad not exists");

    co_await current->dispatch(fn, delay);
}

void fb::threads::settimer(const fb::timer_callback& fn, const timespan& duration)
{
    if(this->_threads.empty())
    {
        throw std::runtime_error("cannot set timer. logic thread does not exists");
    }
    else
    {
        for(auto& [key, value] : this->_threads)
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

fb::thread* fb::threads::operator[](uint8_t index) const
{
    return this->at(index);
}

fb::thread* fb::threads::operator[](std::thread::id id) const
{
    return this->at(id);
}