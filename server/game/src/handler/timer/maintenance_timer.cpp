#include <fb/game/handler/timer/maintenance_timer.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <cstdint>
#include <exception>
#include <format>
#include <optional>

using namespace fb::game::handler::timer;

maintenance_timer::maintenance_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> maintenance_timer::handle()
{
    auto world = fb::config<std::optional<uint32_t>>("world");
    if (!world || this->server.characters.size() == 0)
        co_return;

    try
    {
        auto&& resp = co_await this->server.http.get<fb::protocol::internal::response::GetMaintenance>(
            "internal",
            std::format("/maintenance/{}", *world));
        if (resp.error == 0 && resp.active)
            this->server.characters.on_maintenance(resp.message);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to fetch maintenance state: {}", e.what());
    }
}
