#include <fb/game/handler/timer/system_storage_box_timer.h>

using namespace fb::game::handler::timer;

system_storage_box_timer::system_storage_box_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> system_storage_box_timer::handle()
{
    co_await this->server.system_storage.poll_and_deliver();
}
