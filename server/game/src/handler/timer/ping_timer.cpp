#include <fb/game/handler/timer/ping_timer.h>
#include <fb/game/thread_params.h>
#include <fb/logger.h>
#include <random>

using namespace fb::game::handler::timer;
using namespace fb::game;
using namespace std::chrono_literals;

ping_timer::ping_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> ping_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    constexpr auto ping_interval = 5s;

    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();
    if (params == nullptr)
        co_return;

    static thread_local std::mt19937        rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

    co_await params->characters.foreach_async([now, ping_interval, &dist](auto& ch) -> async::task<void> {
        if (ch == nullptr)
            co_return;

        auto socket_ptr = ch->socket_ptr();
        if (socket_ptr == nullptr || !socket_ptr->is_open())
            co_return;

        auto& state   = ch->ping_state();
        auto  elapsed = now - state.last_ping_time;
        if (elapsed < ping_interval)
            co_return;

        if (!state.pong_received)
        {
            fb::logger::info("Disconnecting character {} (no pong within {}s)", ch->name(), 5);
            socket_ptr->close();
            co_return;
        }

        try
        {
            state.token          = dist(rng);
            state.last_ping_time = now;
            state.pong_received  = false;
            co_await ch->ping(state.token);
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("ping_timer error for character {}: {}", ch->id, e.what());
        }
        co_return;
    });

    co_return;
}
