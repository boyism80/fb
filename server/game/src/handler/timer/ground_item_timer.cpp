#include <fb/game/handler/timer/ground_item_timer.h>

#include <fb/game/item.h>
#include <fb/game/map.h>
#include <fb/game/object.h>
#include <fb/game/thread_params.h>
#include <fb/logger.h>

#include <exception>
#include <memory>
#include <thread>
#include <vector>

using namespace fb::game::handler::timer;

ground_item_timer::ground_item_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> ground_item_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    auto current = this->server.now();
    auto expired = std::vector<std::shared_ptr<fb::game::item>>();
    auto view    = params->map_view;
    for (auto& map : *view)
    {
        if (map->active == false)
            continue;

        for (auto& [_, obj] : map->objects)
        {
            if (obj->is(OBJECT_TYPE::ITEM) == false)
                continue;

            auto  item    = std::static_pointer_cast<fb::game::item>(obj);
            auto& dropped = item->dropped_time();
            if (dropped.has_value() == false)
                continue;

            auto lifetime = fb::model::const_value::time::GROUND_ITEM;
            if (item->death_uid().has_value())
                lifetime = lifetime + fb::model::const_value::death_penalty::warmth_time;

            if (current - dropped.value() >= lifetime)
                expired.push_back(item);
        }
    }

    for (auto& item : expired)
    {
        try
        {
            if (item->map() != nullptr)
                co_await item->destroy();
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("ground item cleanup failed (oid={}): {}", item->oid(), e.what());
        }
    }
    co_return;
}
