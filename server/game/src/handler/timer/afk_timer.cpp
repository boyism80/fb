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
    constexpr auto idle_threshold  = 10min;
    constexpr auto action_interval = 10s;

    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    co_await params->characters.foreach_async([now, idle_threshold, action_interval](auto& ch) -> async::task<void> {
        if (ch == nullptr)
            co_return;

        auto socket_ptr = ch->socket_ptr();
        if (socket_ptr == nullptr || !socket_ptr->is_open())
            co_return;

        const auto& last_packet_time = socket_ptr->last_packet_time();
        auto        elapsed          = now - last_packet_time;
        if (elapsed < idle_threshold)
            co_return;

        auto action_elapsed = now - ch->last_afk_time();
        if (action_elapsed < action_interval)
            co_return;

        try
        {
            co_await ch->action(ACTION::SLEEP, DURATION::EMOTION);
            ch->update_last_afk_time();
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("afk_timer error for character {}: {}", ch->id, e.what());
        }
        co_return;
    });

    co_return;
}
