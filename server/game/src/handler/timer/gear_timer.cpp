#include <fb/game/handler/timer/gear_timer.h>

#include <fb/logger.h>

#include <exception>
#include <map>
#include <memory>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

using namespace fb::game::handler::timer;

gear_timer::gear_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> gear_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();
    auto lua    = this->server.lua.open();

    auto view = params->map_view;
    for (auto& map : *view)
    {
        if (map->active == false)
            continue;

        if (map->objects.size() == 0)
            continue;

        // Scripts may leave us on another thread; resume map walk on this timer thread.
        co_await thread->switching();

        using equipment_map = std::map<EQUIPMENT_PARTS, std::shared_ptr<fb::game::equipment>>;
        auto concast        = std::vector<std::pair<std::weak_ptr<fb::game::character>, equipment_map>>{};
        for (auto& [fd, obj] : map->objects)
        {
            if (obj->is(OBJECT_TYPE::CHARACTER) == false)
                continue;

            auto ch = std::static_pointer_cast<fb::game::character>(obj);
            concast.push_back({std::weak_ptr<fb::game::character>{ch}, ch->items.equipments()});
        }

        for (auto& [weak, equipments] : concast)
        {
            for (auto& [part, equipment] : equipments)
            {
                if (equipment == nullptr)
                    continue;

                auto ch = weak.lock();
                if (ch == nullptr)
                    break;

                if (ch->thread() != thread)
                    break;

                if (ch->state() == STATE::GHOST)
                    break;

                if (ch->items.equipment(part) != equipment)
                    continue;

                if (!lua)
                {
                    lua = this->server.lua.open();
                    if (!lua)
                        continue;
                }

                try
                {
                    auto& model = equipment->model();
                    if (lua->load("scripts/item/{}.lua", model.id) == false || lua->func("on_concast") == false)
                        continue;

                    lua->pushobject(ch);
                    lua->pushobject(equipment);
                    std::ignore = co_await lua->call(2);
                }
                catch (std::exception& e)
                {
                    lua = fb::lua::context::guard{};
                    fb::logger::warn("gear_timer: on_concast failed (map={}): {}", map->id, e.what());
                }
                catch (...)
                {
                    lua = fb::lua::context::guard{};
                    fb::logger::warn("gear_timer: on_concast failed (map={})", map->id);
                }

                co_await thread->switching();
            }
        }
    }

    co_await thread->switching();
    co_return;
}
