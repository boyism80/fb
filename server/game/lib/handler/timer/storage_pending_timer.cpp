#include <fb/game/handler/timer/storage_pending_timer.h>

using namespace fb::game::handler::timer;

storage_pending_timer::storage_pending_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{
}

async::task<void> storage_pending_timer::handle()
{
    co_await this->server.fetch_storage_pending();
    co_return;
}

