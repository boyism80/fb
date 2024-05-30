#include <fb/core/thread.h>

fb::timer::timer(const fb::thread_callback& fn, std::chrono::steady_clock::duration duration) : 
    fn(fn),
    duration(duration)
{ }

fb::timer::~timer()
{ }


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
        auto fn = fb::queue_callback();
        auto awaiter = (fb::awaiter<void>*)nullptr;

        while(this->_precedence.dequeue(fn))
        {
            fn(index);
        }

        while(this->_precedence_awaiter_queue.dequeue(awaiter))
        {
            awaiter->handler.resume();
        }

        if(this->_queue.dequeue(fn))
        {
            fn(index);
        }
        else if(this->_dispatch_awaiter_queue.dequeue(awaiter))
        {
            awaiter->handler.resume();
        }
        else
        {
            auto begin = now();
            this->handle_idle();
            auto elapsed = now() - begin;

            if(elapsed < term)
                std::this_thread::sleep_for(term - elapsed);
        }
    }
}

void fb::thread::handle_idle()
{
    auto copies = std::vector<std::shared_ptr<fb::timer>>(this->_timers);
    auto expired = std::vector<std::shared_ptr<fb::timer>>();
    for(auto& timer : copies)
    {
        auto elapsed = std::chrono::steady_clock::now() - timer->begin;
        if(timer->duration > elapsed)
            continue;

        timer->fn(fb::thread::now(), this->_thread.get_id());
        expired.push_back(timer);
    }

this->_mutex_timer.lock();
    for(auto& x : expired)
    {
        auto found = std::find(this->_timers.begin(), this->_timers.end(), x);
        if(found != this->_timers.end())
            this->_timers.erase(found);
    }
this->_mutex_timer.unlock();
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

void fb::thread::dispatch(const std::function<void()>& fn, const std::chrono::steady_clock::duration& duration)
{   MUTEX_GUARD(this->_mutex_timer)

    // private 생성자때문에 make_shared 사용 X
    auto timer = new fb::timer([fn] (std::chrono::steady_clock::duration, std::thread::id) { fn(); }, duration);
    auto ptr   = std::shared_ptr<fb::timer>(timer);
    this->_timers.push_back(std::move(ptr));
}

void fb::thread::settimer(fb::thread_callback fn, const std::chrono::steady_clock::duration& duration)
{
    this->dispatch
    (
        [this, fn, duration]
        {
            fn(fb::thread::now(), this->_thread.get_id());
            this->settimer(fn, duration);
        }, 
        duration
    );
}

void fb::thread::dispatch(fb::queue_callback&& fn, bool precedence)
{
    if(precedence)
        this->_precedence.enqueue(std::move(fn));
    else
        this->_queue.enqueue(std::move(fn));
}

fb::awaiter<void> fb::thread::precedence()
{
    auto await_callback = [this](auto& awaiter)
    {
        this->_precedence_awaiter_queue.enqueue(&awaiter);
    };

    return fb::awaiter<void>(await_callback);
}

fb::awaiter<void> fb::thread::sleep(const std::chrono::steady_clock::duration& duration)
{
    auto await_callback = [this, duration](auto& awaiter)
    {
        this->dispatch([this, &awaiter]
        {
            this->_dispatch_awaiter_queue.enqueue(&awaiter);
        }, duration);
    };
    return fb::awaiter<void>(await_callback);
}

fb::awaiter<void> fb::thread::dispatch()
{
    auto await_callback = [this](auto& awaiter)
    {
        this->_dispatch_awaiter_queue.enqueue(&awaiter);
    };

    return fb::awaiter<void>(await_callback);
}

std::chrono::steady_clock::duration fb::thread::now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
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

void fb::threads::dispatch(const std::function<void()>& fn, const std::chrono::steady_clock::duration& duration, bool main)
{
    auto current = this->current();
    if(main || current == nullptr)
    {
        auto& context = this->_context;
        auto  timer = std::make_unique<boost::asio::steady_timer>(this->_context, duration);
        timer->async_wait
        (
            [&context, fn, timer = std::move(timer)] (const auto& e)
            {
                boost::asio::dispatch(context, fn);
            }
        );
    }
    else
    {
        current->dispatch(fn, duration);
    }
}

void fb::threads::settimer(fb::thread_callback fn, const std::chrono::steady_clock::duration& duration)
{
    if(this->_threads.empty())
    {
        this->dispatch
        (
            [this, fn, duration]
            {
                fn(fb::thread::now(), std::this_thread::get_id());
                this->settimer(fn, duration);
            },
            duration
        );
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