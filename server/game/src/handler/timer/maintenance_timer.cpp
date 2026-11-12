#include <fb/game/handler/timer/maintenance_timer.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <cstdint>
#include <exception>
#include <format>
#include <optional>
#include <string>

using namespace fb::game::handler::timer;

maintenance_timer::maintenance_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> maintenance_timer::handle()
{
    auto world = fb::config<std::optional<uint32_t>>("world");
    auto path  = world.has_value() ? std::format("/maintenance/{}", world.value()) : std::string("/maintenance");

    auto state = maintenance_state{};
    try
    {
        auto&& resp =
            co_await this->server.http.get<fb::protocol::internal::response::GetMaintenance>("internal", path);
        if (resp.error != 0)
            co_return;

        if (resp.home.has_value())
            state.home = maintenance_window{resp.home->message, resp.home->finish_matches};
        if (resp.cross.has_value())
            state.cross = maintenance_window{resp.cross->message, resp.cross->finish_matches};
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to fetch maintenance state: {}", e.what());
        co_return;
    }

    this->server.maintenance.write([&state](maintenance_state& value) {
        value = state;
    });

    if (state.home.has_value() || state.cross.has_value())
    {
        auto home = world.has_value();
        this->server.characters.foreach_enqueue([state, home](auto& ch) -> async::task<void> {
            if (state.home.has_value())
            {
                if (state.home->admits(ch->role(), ch->matchmaker.session() != nullptr))
                    co_return;

                auto socket_ptr = ch->socket_ptr();
                if (socket_ptr == nullptr)
                    co_return;

                ch->message(state.home->message, MESSAGE_TYPE::STATE);
                socket_ptr->close();
            }
            else if (home && ch->matchmaker.ticket_id().has_value() && state.cross->admits(ch->role(), false) == false)
            {
                co_await ch->matchmaker.drop_ticket("교차 서버 점검으로 매치메이킹 대기가 취소되었습니다.");
            }
        });
    }
}
