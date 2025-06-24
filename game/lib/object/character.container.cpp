#include <fb/game/character.h>
#include <fb/game/context.h>

using namespace fb::game;

character::container::container(fb::game::context& context) :
    _context(context)
{ }

void character::container::insert(std::shared_ptr<character> ch)
{
    this->_from_uid[ch->id()]    = ch;
    this->_from_name[ch->name()] = ch;
}

void character::container::remove(std::shared_ptr<character> ch)
{
    this->_from_uid.erase(ch->id());
    this->_from_name.erase(ch->name());
}

std::shared_ptr<character> character::container::find(uint32_t uid) const
{
    auto it = this->_from_uid.find(uid);
    return it != this->_from_uid.end() ? it->second : nullptr;
}

bool character::container::contains(const std::string& name) const
{
    return this->_from_name.contains(name);
}

bool character::container::contains(uint32_t uid) const
{
    return this->_from_uid.contains(uid);
}

std::shared_ptr<character> character::container::find(const std::string& name) const
{
    auto it = this->_from_name.find(name);
    return it != this->_from_name.end() ? it->second : nullptr;
}

async::task<void> character::container::foreach (std::function<void(std::shared_ptr<character>&)>&&     fn,
                                                 std::function<bool(const std::shared_ptr<character>&)> predict)
{
    co_await this->foreach_async(
        [fn](std::shared_ptr<character>& ch) -> async::task<void> {
            fn(ch);
            co_return;
        },
        std::move(predict));
}

async::task<void>
character::container::foreach_async(std::function<async::task<void>(std::shared_ptr<character>&)>&& fn,
                                    std::function<bool(const std::shared_ptr<character>&)>          predict)
{
    auto targets = std::vector<std::shared_ptr<character>>();
    for (auto& [uid, ch] : this->_from_uid)
    {
        if (predict == nullptr || predict(ch) == false)
            continue;

        targets.push_back(ch);
    }

    co_await this->foreach_async(std::move(fn), std::move(targets));
}

async::task<void> character::container::foreach (std::function<void(std::shared_ptr<character>&)>&& fn,
                                                 const std::vector<std::shared_ptr<character>>&     characters)
{
    co_await this->foreach_async(
        [fn](std::shared_ptr<character>& ch) -> async::task<void> {
            fn(ch);
            co_return;
        },
        std::move(characters));
}

async::task<void>
character::container::foreach_async(std::function<async::task<void>(std::shared_ptr<character>&)>&& fn,
                                    const std::vector<std::shared_ptr<character>>&                  characters)
{
    auto thread = this->_context.threads.current();
    auto group  = std::unordered_map<fb::thread*, std::vector<std::weak_ptr<character>>>();

    for (auto& ch : characters)
    {
        auto each_thread = ch->thread();
        auto weak_ptr    = ch->weak_from_this_as<character>();
        if (group.contains(each_thread) == false)
            group[each_thread] = std::vector<std::weak_ptr<character>>();

        group[each_thread].push_back(weak_ptr);
    }

    for (auto& [thread, weak_ptrs] : group)
    {
        co_await thread->switching();
        for (auto& weak_ptr : weak_ptrs)
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            co_await fn(shared_ptr);
        }
    }

    co_await thread->switching();
}