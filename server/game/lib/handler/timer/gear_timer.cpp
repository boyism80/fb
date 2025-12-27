#include <fb/game/handler/timer/gear_timer.h>

using namespace fb::game::handler::timer;

gear_timer::gear_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> gear_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
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
                    co_await this->server.threads.switching(weak);
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("on_gear_timer: {}", e.what());
                }
            }
        }
    }

    if (lua != nullptr)
        lua->release();
    co_return;
}