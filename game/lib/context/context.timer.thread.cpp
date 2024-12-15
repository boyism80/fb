#include "context.h"

using namespace fb::game;

async::task<void> context::handle_mob_action(const datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->data<thread_params>();

    for (auto& [_, map] : params->maps)
    {
        if (map->active == false)
            continue;

        const auto mobs = map->activateds(OBJECT_TYPE::MOB);

        for (auto x : mobs)
        {
            auto mob = static_cast<fb::game::mob*>(x);
            if (mob->alive() == false)
                continue;

            auto target = mob->target();
            if (target == nullptr || map->objects.contains(*target) == false || target->alive() == false)
                mob->target(nullptr);

            if (mob->action())
                continue;

            co_await mob->AI(now);
        }
    }
    co_return;
}

async::task<void> context::handle_mob_respawn(const datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->data<thread_params>();

    for (auto& rezen : params->rezens)
    {
        co_await rezen.spawn(id);
    }
    co_return;
}

async::task<void> context::handle_buff_timer(const datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->data<thread_params>();

    for (auto& [_, map] : params->maps)
    {
        if (map->active == false)
            continue;

        if (map->objects.size() == 0)
            continue;

        for (auto& [fd, obj] : map->objects)
        {
            if (obj.buffs.size() == 0)
                continue;

            auto ended_buffs = std::vector<buff*>();
            for (auto& [id, buff] : obj.buffs)
            {
                buff->time_dec(1);
                if (buff->time() <= 0ms)
                    ended_buffs.push_back(buff);
            }

            for (auto& buff : ended_buffs)
                std::ignore = co_await obj.buffs.remove(buff->model);
        }
    }

    co_return;
}

async::task<void> context::handle_save_timer(const datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->data<thread_params>();

    for (auto& [id, character] : params->characters)
    {
        std::ignore = this->save(*character);
    }

    co_return;
}