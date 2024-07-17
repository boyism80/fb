#include <fb/core/thread.h>


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
        completed = this->_queue.dequeue([this](auto&& x)
        {
            if(!x.t.done())
                this->_queue.enqueue(std::move(x));
            else
                x.awaiter.resume();
        });
        if(completed)
            continue;

        auto begin = now();
        this->handle_idle();
        auto elapsed = now() - begin;

        if (elapsed < term)
            std::this_thread::sleep_for(term - elapsed);
    }
}

void fb::thread::handle_idle()
{
    auto _ = std::lock_guard(this->_mutex_timer);

    auto indices = std::vector<uint32_t>();
    for (auto i = 0; i < this->_timers.size(); i++)
    {
        auto& timer = this->_timers[i];
        auto elapsed = std::chrono::steady_clock::now() - timer->begin;
        if (timer->duration > elapsed)
            continue;

        timer->fn(fb::thread::now(), this->_thread.get_id());
        if (timer->disposable)
            indices.push_back(i);
    }

    for (int i = indices.size() - 1; i >= 0; i--)
    {
        this->_timers.erase(this->_timers.begin() + indices[i]);
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

fb::awaiter<void> fb::thread::dispatch(const std::function<fb::task<void>()>& fn, const std::chrono::steady_clock::duration& delay, int priority)
{
    return fb::awaiter<void>([this, delay, priority, fn](auto& awaiter)
    {
        if(delay > 0s)
        {
            this->settimer([this, priority, &awaiter, fn](auto time, auto id)
            {
                this->_queue.enqueue(fb::thread::task(fn(), awaiter), priority);
            }, delay, true);
        }
        else
        {
            this->_queue.enqueue(fb::thread::task(fn(), awaiter), priority);
        }
    });
}

fb::awaiter<void> fb::thread::dispatch(uint32_t priority)
{
    static auto generator = [] () -> fb::task<void>
    {
        co_return;
    };

    return this->dispatch(generator, 0s, priority);
}

void fb::thread::settimer(const fb::timer_callback& fn, const std::chrono::steady_clock::duration& duration, bool disposable)
{
    auto _ = std::lock_guard(this->_mutex_timer);

    auto timer = new fb::timer([this, fn](std::chrono::steady_clock::duration, std::thread::id)
    {
        fn(fb::thread::now(), this->_thread.get_id());
    }, duration, disposable);
    this->_timers.push_back(std::unique_ptr<fb::timer>(timer));
}

fb::awaiter<void> fb::thread::sleep(const std::chrono::steady_clock::duration& delay)
{
    static auto generator = [] () -> fb::task<void>
    {
        co_return;
    };

    return this->dispatch(generator, delay);
}

std::chrono::steady_clock::duration fb::thread::now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

fb::thread::task::task(fb::task<void>&& t, fb::awaiter<void>& awaiter) : t(std::move(t)), awaiter(awaiter)
{}

fb::thread::task::task(fb::thread::task&& r) : t(std::move(r.t)), awaiter(r.awaiter)
{}

fb::thread::task::~task()
{}

void fb::thread::task::operator = (fb::thread::task&& r)
{
    this->t = std::move(r.t);
    this->awaiter = std::move(r.awaiter);
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

fb::task<void> fb::threads::dispatch(const std::function<fb::task<void>()>& fn, const std::chrono::steady_clock::duration& delay)
{
    auto current = this->current();
    if (current == nullptr)
        throw std::runtime_error("therad not exists");

    co_await current->dispatch(fn, delay);
}

void fb::threads::settimer(const fb::timer_callback& fn, const std::chrono::steady_clock::duration& duration)
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