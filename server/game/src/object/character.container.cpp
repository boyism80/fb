#include <fb/game/character.h>
#include <fb/game/server.h>
#include <atomic>

using namespace fb::game;

character::container::container(fb::game::server& server) :
    _server(server)
{ }

size_t character::container::size() const
{
    return this->_from_uid.size();
}

bool character::container::insert(character_ptr_t ch)
{
    if (this->_from_uid.contains(ch->id) || this->_from_name.contains(ch->name()))
        return false;

    this->_from_uid[ch->id]      = ch;
    this->_from_name[ch->name()] = ch;
    return true;
}

void character::container::remove(character_ptr_t ch)
{
    this->_from_uid.erase(ch->id);
    this->_from_name.erase(ch->name());
}

character::container::character_ptr_t character::container::find(uint32_t uid) const
{
    auto it = this->_from_uid.find(uid);
    return it != this->_from_uid.end() ? it->second : nullptr;
}

bool character::container::contains(std::string_view name) const
{
    return this->_from_name.contains(std::string(name));
}

bool character::container::contains(uint32_t uid) const
{
    return this->_from_uid.contains(uid);
}

character::container::character_ptr_t character::container::find(std::string_view name) const
{
    auto it = this->_from_name.find(std::string(name));
    return it != this->_from_name.end() ? it->second : nullptr;
}

async::task<void> character::container::foreach (character_function_t fn, character_predicate_t predict)
{
    co_await this->foreach_async(
        [fn](character_ptr_t& ch) -> async::task<void> {
            fn(ch);
            co_return;
        },
        std::move(predict));
}

async::task<void> character::container::foreach_async(character_async_function_t fn, character_predicate_t predict)
{
    auto targets = std::vector<character_ptr_t>();
    for (auto& [uid, ch] : this->_from_uid)
    {
        if (predict != nullptr && predict(ch) == false)
            continue;

        targets.push_back(ch);
    }

    co_await this->foreach_async(std::move(fn), std::move(targets));
}

async::task<void> character::container::foreach (character_function_t                fn,
                                                 const std::vector<character_ptr_t>& characters)
{
    co_await this->foreach_async(
        [fn](character_ptr_t& ch) -> async::task<void> {
            fn(ch);
            co_return;
        },
        characters);
}

async::task<void> character::container::foreach_async(character_async_function_t          fn,
                                                      const std::vector<character_ptr_t>& characters)
{
    auto before = this->_server.threads.current();
    auto weak_ptrs = std::vector<std::weak_ptr<character>>();
    weak_ptrs.reserve(characters.size());

    for (auto& ch : characters)
    {
        if (ch == nullptr)
            continue;

        weak_ptrs.push_back(ch->weak_from_this_as<character>());
    }

    if (weak_ptrs.empty())
    {
        if (before != nullptr)
            co_await before->switching();
        co_return;
    }

    auto promise   = std::make_shared<async::task_completion_source<void>>();
    auto remaining = std::make_shared<std::atomic_size_t>(weak_ptrs.size());

    auto fn_holder = std::make_shared<character_async_function_t>(std::move(fn));
    for (auto& weak_ptr : weak_ptrs)
    {
        async::awaitable_then(
            this->_server.threads.dispatch(
                weak_ptr,
                [weak_ptr, fn_holder](auto& thread) -> async::task<void> {
                    auto shared_ptr = weak_ptr.lock();
                    if (shared_ptr != nullptr)
                        co_await (*fn_holder)(shared_ptr);
                    co_return;
                }),
            [promise, remaining](async::awaitable_result<void> result) mutable {
                try
                {
                    result();
                }
                catch (std::exception&)
                {
                }
                catch (...)
                {
                }

                // Count down regardless of success/failure to avoid deadlock.
                if (remaining->fetch_sub(1) == 1)
                    promise->set_value();
            });
    }

    co_await promise->task();

    if (before != nullptr)
        co_await before->switching();
}

async::task<void> character::container::foreach (const std::vector<std::string>& names,
                                                 character_function_t            fn,
                                                 character_function_t_miss       miss)
{
    co_await this->foreach_async(
        std::move(names),
        [fn, miss](character_ptr_t& ch) -> async::task<void> {
            fn(ch);
            co_return;
        },
        std::move(miss));
}

async::task<void> character::container::foreach_async(const std::vector<std::string>& names,
                                                      character_async_function_t      fn,
                                                      character_function_t_miss       miss)
{
    auto targets = std::vector<character_ptr_t>();
    for (auto& name : names)
    {
        auto ch = this->find(name);
        if (ch == nullptr)
        {
            if (miss != nullptr)
                miss(name);
            continue;
        }

        targets.push_back(ch);
    }

    co_await this->foreach_async(std::move(fn), std::move(targets));
}

async::task<void> character::container::invoke(std::string_view          name,
                                               character_function_t      fn,
                                               character_function_t_miss miss)
{
    auto ch = this->find(name);
    if (ch == nullptr)
    {
        if (miss != nullptr)
            miss(name);
        co_return;
    }

    auto before = this->_server.threads.current();
    auto weak   = ch->weak_from_this_as<character>();
    co_await this->_server.threads.switching(weak);
    auto ptr = weak.lock();
    if (ptr != nullptr)
        fn(ptr);
    if (before != nullptr)
        co_await before->switching();
}

async::task<void> character::container::invoke_async(std::string_view           name,
                                                     character_async_function_t fn,
                                                     character_function_t_miss  miss)
{
    auto ch = this->find(name);
    if (ch == nullptr)
    {
        if (miss != nullptr)
            miss(name);
        co_return;
    }

    auto before = this->_server.threads.current();
    auto weak   = ch->weak_from_this_as<character>();
    co_await this->_server.threads.switching(weak);
    auto ptr = weak.lock();
    if (ptr != nullptr)
        co_await fn(ptr);
    if (before != nullptr)
        co_await before->switching();
}

void character::container::foreach_enqueue(character_async_function_t&&        fn,
                                           const std::vector<character_ptr_t>& characters)
{
    auto group = std::unordered_map<fb::thread*, std::vector<std::weak_ptr<character>>>();

    for (auto& ch : characters)
    {
        auto each_thread = ch->thread();
        if (each_thread == nullptr)
            continue;

        auto weak_ptr = ch->weak_from_this_as<character>();
        if (group.contains(each_thread) == false)
            group[each_thread] = std::vector<std::weak_ptr<character>>();

        group[each_thread].push_back(weak_ptr);
    }

    auto fn_holder = std::make_shared<character_async_function_t>(std::move(fn));
    for (auto& [thread, weak_ptrs] : group)
    {
        thread->enqueue(
            [fn_holder, weak_ptrs](auto& thread) -> async::task<void> {
                for (auto& weak_ptr : weak_ptrs)
                {
                    auto shared_ptr = weak_ptr.lock();
                    if (shared_ptr == nullptr)
                        continue;

                    co_await (*fn_holder)(shared_ptr);
                }
            },
            [](std::exception& e) {
                fb::logger::fatal("foreach_enqueue error: {}", e.what());
            },
            []() {
                // work done
            });
    }
}

void character::container::foreach_enqueue(character_async_function_t&& fn, character_predicate_t predict)
{
    auto targets = std::vector<character_ptr_t>();
    for (auto& [uid, ch] : this->_from_uid)
    {
        if (predict != nullptr && predict(ch) == false)
            continue;

        targets.push_back(ch);
    }

    this->foreach_enqueue(std::move(fn), std::move(targets));
}

void character::container::foreach_enqueue(const std::vector<std::string>& names,
                                           character_async_function_t&&    fn,
                                           character_function_t_miss       miss)
{
    auto targets = std::vector<character_ptr_t>();
    for (auto& name : names)
    {
        auto ch = this->find(name);
        if (ch == nullptr)
        {
            if (miss != nullptr)
                miss(name);
            continue;
        }

        targets.push_back(ch);
    }

    this->foreach_enqueue(std::move(fn), std::move(targets));
}

character::container::character_ptr_t character::container::operator[] (uint32_t uid)
{
    auto ch = this->find(uid);
    if (ch == nullptr)
        throw std::out_of_range("out of range exception");

    return ch;
}

character::container::character_ptr_t character::container::operator[] (std::string_view name)
{
    auto ch = this->find(name);
    if (ch == nullptr)
        throw std::out_of_range("out of range exception");

    return ch;
}

character::container::iterator character::container::begin()
{
    return this->_from_uid.begin();
}

character::container::iterator character::container::end()
{
    return this->_from_uid.end();
}

character::container::const_iterator character::container::begin() const
{
    return this->_from_uid.begin();
}

character::container::const_iterator character::container::end() const
{
    return this->_from_uid.end();
}

character::container::const_iterator character::container::cbegin() const
{
    return this->_from_uid.cbegin();
}

character::container::const_iterator character::container::cend() const
{
    return this->_from_uid.cend();
}