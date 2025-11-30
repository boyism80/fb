#include <fb/gateway/handler/timer/log_flush.h>

using namespace fb::gateway::handler::timer;

log_flush::log_flush(fb::gateway::server& server) :
    fb::handler::timer<fb::gateway::server>(server)
{ }

async::task<void> log_flush::handle()
{
    if (this->server.log != nullptr)
    {
        co_await this->server.log->flush();
    }
    co_return;
}
