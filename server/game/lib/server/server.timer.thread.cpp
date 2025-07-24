#include <fb/game/server.h>

using namespace fb::game;

async::task<void> server::handle_mob_action(const fb::model::datetime& now, std::thread::id id)
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
            if (obj->is(OBJECT_TYPE::MOB) == false)
                continue;

            auto mob = std::static_pointer_cast<fb::game::mob>(obj);
            if (mob->alive() == false)
                continue;

            if (mob->paralysis())
                continue;

            std::ignore = mob->action(now);
        }
    }
    co_return;
}

async::task<void> server::handle_mob_respawn(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& rezen : params->rezens)
    {
        std::ignore = rezen.spawn(id);
    }
    co_return;
}

async::task<void> server::handle_buff_timer(const fb::model::datetime& now, std::thread::id id)
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
            if (obj->buffs.size() == 0)
                continue;

            concast.push_back(obj.get());
        }

        for (auto obj : concast)
        {
            auto ended_buffs = std::vector<std::shared_ptr<fb::game::buff>>();
            auto buffs       = obj->buffs; // To avoid iterator invalidation
            for (auto& [id, buff] : buffs)
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
                    lua->func(buff->model.concast);
                    lua->pushobject(obj);
                    if (buff->caster == nullptr)
                        lua->pushnil();
                    else
                        lua->pushobject(buff->caster);
                    lua->pushobject(buff);
                    std::ignore = co_await lua->call(3);
                    continue;
                }
            }

            for (auto& buff : ended_buffs)
                std::ignore = obj->buffs.remove(buff->model);
        }
    }

    co_return;
}

async::task<void> server::handle_gear_timer(const fb::model::datetime& now, std::thread::id id)
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

        auto concast = std::unordered_map<fb::game::character*, std::vector<std::shared_ptr<fb::game::equipment>>>{};
        for (auto& [fd, obj] : map->objects)
        {
            if (obj->is(OBJECT_TYPE::CHARACTER) == false)
                continue;

            auto ch = std::static_pointer_cast<fb::game::character>(obj);
            if (ch->state() == STATE::GHOST)
                continue;

            for (auto& [part, equipment] : ch->items.equipments())
            {
                if (equipment == nullptr)
                    continue;

                auto& model = equipment->based<fb::model::equipment>();
                if (model.on_concast.empty())
                    continue;

                if (concast.contains(ch.get()) == false)
                    concast.insert({ch.get(), {}});
                concast[ch.get()].push_back(equipment);
            }
        }

        for (auto& [ch, equipments] : concast)
        {
            auto weak = ch->weak_from_this();
            for (auto& equipment : equipments)
            {
                try
                {
                    auto& model = equipment->based<fb::model::equipment>();
                    if (lua != nullptr)
                    {
                        lua->func(model.on_concast);
                        lua->pushobject(ch);
                        lua->pushobject(equipment);
                    }
                    std::ignore = co_await lua->call(2, false);
                    co_await this->threads.switching(weak);
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("handle_gear_timer: {}", e.what());
                }
            }
        }
    }

    if (lua != nullptr)
        lua->release();
    co_return;
}

async::task<void> fb::game::server::handle_soliloquy_timer(const fb::model::datetime& now, std::thread::id id)
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
            if (obj->is(OBJECT_TYPE::NPC) == false)
                continue;

            auto npc = std::static_pointer_cast<fb::game::npc>(obj);
            npc->soliloquy();
        }
    }

    co_return;
}

async::task<void> server::handle_save_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->threads.at(id);
    auto params = thread->template data<thread_params>();

    for (auto& [id, character] : params->characters)
    {
        std::ignore = this->save(*character);
    }

    co_return;
}
