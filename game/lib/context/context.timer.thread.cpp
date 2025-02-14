#include <fb/game/context.h>

using namespace fb::game;

async::task<void> context::handle_mob_action(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [_, map] : params->maps)
    {
        if (map->active == false)
            continue;

        if (map->is_active() == false)
            continue;

        for (auto& [_, obj] : map->objects)
        {
            if (obj.is(OBJECT_TYPE::MOB) == false)
                continue;

            auto& mob = static_cast<fb::game::mob&>(obj);
            if (mob.alive() == false)
                continue;

            if (mob.paralysis())
                continue;

            auto target = mob.target();
            if (target == nullptr || map->objects.contains(*target) == false || target->alive() == false)
                mob.target(nullptr);

            if (mob.action())
                continue;

            mob.AI(now);
        }
    }
    co_return;
}

async::task<void> context::handle_mob_respawn(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& rezen : params->rezens)
    {
        rezen.spawn(id);
    }
    co_return;
}

async::task<void> context::handle_buff_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [_, map] : params->maps)
    {
        if (map->active == false)
            continue;

        if (map->objects.size() == 0)
            continue;

        auto concast = std::vector<fb::game::object*>{};
        for (auto& [fd, obj] : map->objects)
        {
            if (obj.buffs.size() == 0)
                continue;

            concast.push_back(&obj);
        }

        for (auto obj : concast)
        {
            auto ended_buffs = std::vector<buff*>();
            for (auto& [id, buff] : obj->buffs)
            {
                buff->time_dec(1s);
                if (buff->time() <= 0ms)
                {
                    ended_buffs.push_back(buff);
                    continue;
                }

                if (buff->model.concast.empty() == false)
                {
                    auto lua = lua::new_context();
                    lua->from(buff->model.concast.c_str()).func("on_concast").pushobject(obj);
                    if (buff->caster == nullptr)
                        lua->pushnil();
                    else
                        lua->pushobject(buff->caster);
                    lua->pushobject(buff);
                    lua->resume(3);
                    continue;
                }
            }

            for (auto& buff : ended_buffs)
                std::ignore = obj->buffs.remove(buff->model);
        }
    }

    co_return;
}

async::task<void> context::handle_gear_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->template data<thread_params>();
    auto lua    = fb::lua::new_context();

    for (auto& [_, map] : params->maps)
    {
        if (map->active == false)
            continue;

        if (map->objects.size() == 0)
            continue;

        auto concast = std::unordered_map<fb::game::character*, std::vector<fb::game::equipment*>>{};
        for (auto& [fd, obj] : map->objects)
        {
            if (obj.is(OBJECT_TYPE::CHARACTER) == false)
                continue;

            auto& ch = static_cast<fb::game::character&>(obj);
            for (auto& [part, item] : ch.items.equipments())
            {
                if (item == nullptr)
                    continue;

                auto  equipment = static_cast<fb::game::equipment*>(item);
                auto& model     = equipment->based<fb::model::equipment>();
                if (model.script_concast == "")
                    continue;

                if (concast.contains(&ch) == false)
                    concast.insert({&ch, {}});

                concast[&ch].push_back(equipment);
            }
        }

        for (auto& [ch, equipments] : concast)
        {
            for (auto equipment : equipments)
            {
                auto& model = equipment->based<fb::model::equipment>();
                lua->from(model.script_concast);
                lua->func("on_concast");
                lua->pushobject(ch);
                lua->pushobject(equipment);
                lua->resume(2, false);
            }
        }
    }

    lua->release();
    co_return;
}

async::task<void> context::handle_save_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [id, character] : params->characters)
    {
        std::ignore = this->save(*character);
    }

    co_return;
}