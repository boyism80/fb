#include <fb/game/handler/timer/maintenance_timer.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <optional>
#include <string>
#include <string_view>

using namespace fb::game::handler::timer;

maintenance_timer::maintenance_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> maintenance_timer::handle()
{
    auto world = fb::config<std::optional<uint32_t>>("world");
    auto label = world.has_value() ? std::format("world:{}", world.value()) : std::string("cross");
    auto path  = world.has_value() ? std::format("/maintenance/{}", world.value()) : std::string("/maintenance");

    auto state = maintenance_state{};
    try
    {
        auto&& resp =
            co_await this->server.http.get<fb::protocol::internal::response::GetMaintenance>("internal", path);
        if (resp.error != 0)
            co_return;

        for (const auto& maintenance : resp.maintenances)
        {
            auto& labels  = maintenance.labels;
            auto  targets = [&labels](std::string_view x) {
                return std::find(labels.begin(), labels.end(), "all") != labels.end() ||
                       std::find(labels.begin(), labels.end(), x) != labels.end();
            };

            if (targets(label))
            {
                if (state.active)
                {
                    state.finish_matches = state.finish_matches && maintenance.finish_matches;
                }
                else
                {
                    state.active         = true;
                    state.message        = maintenance.message;
                    state.finish_matches = maintenance.finish_matches;
                }
            }

            if (targets("cross"))
                state.cross = true;
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to fetch maintenance state: {}", e.what());
        co_return;
    }

    this->server.maintenance.write([&state](maintenance_state& value) {
        value = state;
    });
    this->server.characters.on_maintenance(state);
}
