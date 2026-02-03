#include <fb/thread_container.h>
#include <fb/async_executor.h>

using namespace fb;

thread_container::thread_container(fb::async_executor& executor, uint32_t count) :
    _executor(executor)
{
    if (count > 0)
        this->_keys = std::make_unique<std::thread::id[]>(count);

    for (int i = 0; i < count; i++)
    {
        auto ptr       = std::make_unique<thread>(i);
        auto id        = ptr->id();
        this->_keys[i] = id;
        this->_logic_threads.insert({id, std::move(ptr)});
    }
}

thread_container::~thread_container()
{
    // std::unique_ptr in thread::_data handles cleanup automatically

    for (auto& [id, thread] : this->_logic_threads)
    {
        thread->exit();
        thread->join();
    }
}

thread* thread_container::at(uint8_t index) const
{
    if (this->_logic_threads.size() == 0)
        return nullptr;

    if (index > this->_logic_threads.size() - 1)
        return nullptr;

    auto& id = this->_keys[index];
    return this->at(id);
}

thread* thread_container::at(std::thread::id id) const
{
    auto found = this->_logic_threads.find(id);
    if (found == this->_logic_threads.end())
        return nullptr;
    else
        return found->second.get();
}

thread* thread_container::modular(uint32_t id) const
{
    if (this->_logic_threads.size() == 0)
        return nullptr;

    auto index = id % this->size();
    return this->at(index);
}

thread* thread_container::current()
{
    auto id    = std::this_thread::get_id();
    auto found = this->_logic_threads.find(id);
    if (found == this->_logic_threads.end())
        return nullptr;
    else
        return found->second.get();
}

const thread* thread_container::current() const
{
    auto id    = std::this_thread::get_id();
    auto found = this->_logic_threads.find(id);
    if (found == this->_logic_threads.end())
        return nullptr;
    else
        return found->second.get();
}

uint8_t thread_container::count() const
{
    return (uint8_t)this->_logic_threads.size();
}

bool thread_container::empty() const
{
    return this->_logic_threads.size() == 0;
}

bool thread_container::valid(uint8_t index) const
{
    auto current_thread = this->current();
    if (current_thread == nullptr)
        return false;
    return current_thread->index() == index;
}

bool thread_container::valid(const fb::thread& thread) const
{
    auto current_thread = this->current();
    if (current_thread == nullptr)
        return false;
    return current_thread->id() == thread.id();
}

bool thread_container::valid(thread* thread) const
{
    if (thread == nullptr)
        return this->empty();

    return this->valid(*thread);
}

size_t thread_container::size() const
{
    return this->_logic_threads.size();
}

void thread_container::settimer(fb::timer::handle_callback_type&& fn, const fb::model::timespan& duration)
{
    if (this->_logic_threads.empty())
    {
        throw std::runtime_error("cannot set timer. logic thread does not exists");
    }
    else
    {
        for (auto& [key, thread] : this->_logic_threads)
        {
            std::ignore = thread->dispatch([fn = std::move(fn), duration](auto& thread) mutable -> async::task<void> {
                thread.settimer(std::move(fn), duration);
                co_return;
            });
        }
    }
}

void thread_container::exit()
{
    for (auto& [id, thread] : this->_logic_threads)
    {
        thread->exit();
    }
}

thread* thread_container::least_loaded() const
{
    if (this->_logic_threads.empty())
        return nullptr;

    thread* result   = nullptr;
    size_t  min_size = SIZE_MAX;

    for (auto& [id, thread] : this->_logic_threads)
    {
        auto size = thread->queue_size();
        if (size < min_size)
        {
            min_size = size;
            result   = thread.get();
        }
    }

    return result;
}

thread* thread_container::operator[] (uint8_t index) const
{
    return this->at(index);
}

thread* thread_container::operator[] (std::thread::id id) const
{
    return this->at(id);
}

thread_container::iterator thread_container::begin()
{
    return this->_logic_threads.begin();
}

thread_container::iterator thread_container::end()
{
    return this->_logic_threads.end();
}

thread_container::const_iterator thread_container::begin() const
{
    return this->_logic_threads.begin();
}

thread_container::const_iterator thread_container::end() const
{
    return this->_logic_threads.end();
}

thread_container::const_iterator thread_container::cbegin() const
{
    return this->_logic_threads.cbegin();
}

thread_container::const_iterator thread_container::cend() const
{
    return this->_logic_threads.cend();
}