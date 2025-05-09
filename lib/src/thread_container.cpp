#include <fb/thread_container.h>
#include <fb/abstract.h>

using namespace fb;

thread_container::thread_container(fb::context& context, uint32_t count) :
    _context(context)
{
    if (count > 0)
        this->_keys = std::make_unique<std::thread::id[]>(count);

    for (int i = 0; i < count; i++)
    {
        auto ptr       = std::make_unique<thread>(i);
        auto id        = ptr->id();
        this->_keys[i] = id;
        this->_thread_container.insert({id, std::move(ptr)});
    }
}

void thread_container::enqueue(thread_switchable&                          pivot,
                               const std::function<bool(fb::thread&)>&     condition,
                               const thread::handle_func_type<void>&       fn,
                               const std::function<void(std::exception&)>& error,
                               const std::function<void()>&                callback)
{
    auto thread = pivot.thread();
    if (thread == nullptr)
        throw std::runtime_error("no matched thread");

    thread->enqueue(
        [=, &pivot, this](auto& thread) -> async::task<void> {
            if (condition(thread) == false)
                throw std::runtime_error("condition not satisfied");

            if (this->_context.alive(pivot) == false)
                throw std::runtime_error("pivot is not alive");

            auto active_thread = pivot.thread();
            if (active_thread != &thread)
            {
                this->enqueue(pivot, condition, fn);
            }
            else
            {
                co_await fn(*active_thread);
            }
        },
        error,
        callback);
}

void thread_container::enqueue(thread_switchable&                      pivot,
                               const std::function<bool(fb::thread&)>& condition,
                               const thread::handle_func_type<void>&   fn)
{
    return this->enqueue(
        pivot,
        condition,
        fn,
        [](std::exception& e) {
        },
        []() {
        });
}

void thread_container::enqueue(thread_switchable& pivot, const thread::handle_func_type<void>& fn)
{
    return this->enqueue(
        pivot,
        [](auto& thread) -> bool {
            return true;
        },
        fn,
        [](std::exception& e) {
        },
        []() {
        });
}

async::task<void> thread_container::dispatch(thread_switchable&                      pivot,
                                             const std::function<bool(fb::thread&)>& condition,
                                             const thread::handle_func_type<void>&   fn)
{
    auto promise = std::make_shared<async::task_completion_source<void>>();
    this->enqueue(
        pivot,
        condition,
        fn,
        [promise](std::exception& e) {
            promise->set_exception(std::make_exception_ptr(e));
        },
        [promise]() {
            promise->set_value();
        });
    return promise->task();
}

async::task<void> thread_container::dispatch(thread_switchable& pivot, const thread::handle_func_type<void>& fn)
{
    auto promise = std::make_shared<async::task_completion_source<void>>();
    this->enqueue(
        pivot,
        [](auto& thread) -> bool {
            return true;
        },
        fn,
        [promise](std::exception& e) {
            promise->set_exception(std::make_exception_ptr(e));
        },
        [promise]() {
            promise->set_value();
        });
    return promise->task();
}

async::task<void> thread_container::switching(thread_switchable& pivot)
{
    co_await this->dispatch(pivot, [](auto&) -> async::task<void> {
        co_return;
    });
}

thread* thread_container::at(uint8_t index) const
{
    if (this->_thread_container.size() == 0)
        return nullptr;

    if (index > this->_thread_container.size() - 1)
        return nullptr;

    auto& id = this->_keys[index];
    return this->at(id);
}

thread* thread_container::at(std::thread::id id) const
{
    auto found = this->_thread_container.find(id);
    if (found == this->_thread_container.end())
        return nullptr;
    else
        return found->second.get();
}

thread* thread_container::modular(uint32_t id) const
{
    if (this->_thread_container.size() == 0)
        return nullptr;

    auto index = id % this->size();
    return this->at(index);
}

thread* thread_container::current()
{
    auto id    = std::this_thread::get_id();
    auto found = this->_thread_container.find(id);
    if (found == this->_thread_container.end())
        return nullptr;
    else
        return found->second.get();
}

const thread* thread_container::current() const
{
    auto id    = std::this_thread::get_id();
    auto found = this->_thread_container.find(id);
    if (found == this->_thread_container.end())
        return nullptr;
    else
        return found->second.get();
}

uint8_t thread_container::count() const
{
    return (uint8_t)this->_thread_container.size();
}

bool thread_container::empty() const
{
    return this->_thread_container.size() == 0;
}

bool thread_container::valid(uint8_t index) const
{
    auto current = this->current();
    return current->index() == index;
}

bool thread_container::valid(thread* thread) const
{
    if (thread == nullptr)
        return this->empty();

    return this->valid(*thread);
}

bool thread_container::valid(thread& thread) const
{
    auto current = this->current();
    return current != nullptr && current->id() == thread.id();
}

size_t thread_container::size() const
{
    return this->_thread_container.size();
}

void thread_container::settimer(const timer::handle_callback_type& fn, const model::timespan& duration)
{
    if (this->_thread_container.empty())
    {
        throw std::runtime_error("cannot set timer. logic thread does not exists");
    }
    else
    {
        for (auto& [key, value] : this->_thread_container)
        {
            value->settimer(fn, duration);
        }
    }
}

void thread_container::exit()
{
    for (auto& [id, thread] : this->_thread_container)
    {
        thread->exit();
    }

    this->_thread_container.clear();
}

thread* thread_container::operator[] (uint8_t index) const
{
    return this->at(index);
}

thread* thread_container::operator[] (std::thread::id id) const
{
    return this->at(id);
}