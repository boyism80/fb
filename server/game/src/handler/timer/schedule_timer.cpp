#include <fb/game/handler/timer/schedule_timer.h>
#include <fb/game/server.h>
#include <fb/amqp_route.h>

using namespace fb::game::handler::timer;

schedule_timer::schedule_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> schedule_timer::handle()
{
    if (!fb::config<std::optional<uint32_t>>("world"))
        co_return;

    co_await this->server.schedules.poll();
}
