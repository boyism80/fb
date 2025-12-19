#include <fb/game/handler/timer/afk_timer.h>
#include <fb/game/thread_params.h>
#include <fb/model/model.h>
#include <fb/logger.h>

using namespace fb::game::handler::timer;
using namespace fb::game;
using namespace fb::model::enum_value;
using namespace std::chrono_literals;

afk_timer::afk_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> afk_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
#if defined(DEBUG) || defined(_DEBUG)
    constexpr auto idle_threshold = 30s;
#else
    constexpr auto idle_threshold = 5min;
#endif
    constexpr auto action_interval = 10s;

    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    // Iterate through all characters in this thread
    for (auto& [uid, ch] : params->characters)
    {
        if (ch == nullptr)
            continue;

        // Check if socket is still open
        if (!ch->socket.is_open())
            continue;

        // Calculate elapsed time since last packet
        const auto& last_packet_time = ch->socket.last_packet_time();
        auto        elapsed          = now - last_packet_time;

        // Check if 5 minutes (300 seconds) have passed since last packet
        if (elapsed < idle_threshold)
            continue;

        // Check if 10 seconds have passed since last idle action
        auto action_elapsed = now - ch->last_afk_time();

        if (action_elapsed < action_interval)
            continue;

        // Execute attack action (temporary, will be changed to SLEEP when available)
        try
        {
            ch->action(ACTION::SLEEP, DURATION::EMOTION);
            // Update last idle action time only for timer-triggered actions
            ch->update_last_afk_time();
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("afk_timer error for character {}: {}", ch->id, e.what());
        }
    }

    co_return;
}
