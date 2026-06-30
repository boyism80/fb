#include <fb/game/handler/timer/gear_timer.h>
#include <format>

using namespace fb::game::handler::timer;

gear_timer::gear_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> gear_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();
    auto lua    = this->server.lua.new_context(nullptr, {.auto_release = false});

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

                if (concast.contains(ch.get()) == false)
                    concast.insert({ch.get(), {}});

                auto& model = equipment->based<fb::model::equipment>();
                concast[ch.get()].push_back(equipment);
            }
        }

        for (auto& [ch, equipments] : concast)
        {
            auto weak = ch->weak_from_this();
            for (auto& equipment : equipments)
            {
                if (lua == nullptr)
                {
                    lua = this->server.lua.new_context(nullptr, {.auto_release = false});
                    if (lua == nullptr)
                        continue;
                }

                try
                {
                    auto& model = equipment->based<fb::model::equipment>();
                    auto  path  = std::format("scripts/item/{}.lua", model.id);
                    auto  func  = std::format("ON_CONCAST_{}", model.id);

                    if (lua->load(path) == false)
                        continue;

                    if (lua->func(func) == false)
                        continue;

                    lua->pushobject(ch);
                    lua->pushobject(equipment);

                    try
                    {
                        std::ignore = co_await lua->call(2);
                    }
                    catch (...)
                    {
                        lua = nullptr;
                        throw;
                    }

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
