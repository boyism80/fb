#include <fb/game/handler/timer/buff_timer.h>
#include <format>

using namespace fb::game::handler::timer;

buff_timer::buff_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> buff_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
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
                if (buff->remaining() <= 0ms)
                {
                    ended_buffs.push_back(buff);
                    continue;
                }

                auto& model = buff->model;
                auto  path  = std::format("scripts/spell/{}.lua", model.id);
                auto  func  = std::format("ON_CONCAST_{}", model.id);

                auto lua = this->server.lua.open(path, func);
                if (!lua)
                    continue;

                lua->pushobject(obj);
                if (buff->caster == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(buff->caster);
                lua->pushobject(buff);
                std::ignore = co_await lua->call(3);
            }

            for (auto& buff : ended_buffs)
            {
                std::ignore = obj->buffs.remove(buff->model);
            }
        }
    }

    co_return;
}