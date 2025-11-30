#include <fb/game/handler/timer/log_flush.h>

using namespace fb::game::handler::timer;

log_flush::log_flush(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> log_flush::handle()
{
    if (this->server.log != nullptr)
    {
        co_await this->server.log->flush();
    }
    co_return;
}
