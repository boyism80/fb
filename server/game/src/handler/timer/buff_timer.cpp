#include <fb/game/handler/timer/buff_timer.h>
#include <fb/logger.h>
#include <format>

using namespace fb::game::handler::timer;

buff_timer::buff_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> buff_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    auto view = params->map_view;
    for (auto& map : *view)
    {
        if (map->active == false)
            continue;

        if (map->objects.size() == 0)
            continue;

        // Scripts/remove may leave us on another thread; always resume map walk here.
        if (std::this_thread::get_id() != thread->id())
            co_await thread->switching();

        auto concast = std::vector<fb::game::object*>{};
        for (auto& [fd, obj] : map->objects)
        {
            if (obj->buffs.size() == 0)
                continue;

            concast.push_back(obj.get());
        }

        for (auto obj : concast)
        {
            auto map_id = map->id;
            auto weak   = obj->weak_from_this_as<fb::game::object>();

            if (std::this_thread::get_id() != thread->id())
                co_await thread->switching();

            auto ended_buffs = std::vector<std::shared_ptr<fb::game::buff>>();
            auto buffs       = obj->buffs; // To avoid iterator invalidation
            for (auto& [buff_id, buff] : buffs)
            {
                if (buff->remaining() <= 0ms)
                {
                    ended_buffs.push_back(buff);
                    continue;
                }

                auto& model = buff->model();
                auto  path  = std::format("scripts/spell/{}.lua", model.id);
                auto  func  = "on_concast";

                auto lua = this->server.lua.open(path, func);
                if (!lua)
                    continue;

                lua->pushobject(obj);
                if (buff->caster == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(buff->caster);
                lua->pushobject(buff);

                try
                {
                    std::ignore = co_await lua->call(3);
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("buff_timer: on_concast failed (map={}, spell={}): {}",
                                     map_id,
                                     model.id,
                                     e.what());
                }

                if (std::this_thread::get_id() != thread->id())
                    co_await thread->switching();
            }

            for (auto& buff : ended_buffs)
            {
                try
                {
                    co_await this->server.threads.switching(weak);
                    std::ignore = co_await obj->buffs.remove(buff->model());
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("buff_timer: remove failed (map={}, spell={}): {}",
                                     map_id,
                                     buff->model().id,
                                     e.what());
                }

                if (std::this_thread::get_id() != thread->id())
                    co_await thread->switching();
            }
        }
    }

    if (std::this_thread::get_id() != thread->id())
        co_await thread->switching();

    co_return;
}
