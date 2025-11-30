#include <fb/login/handler/timer/log_flush.h>

using namespace fb::login::handler::timer;

log_flush::log_flush(fb::login::server& server) :
    fb::handler::timer<fb::login::server>(server)
{ }

async::task<void> log_flush::handle()
{
    if (this->server.log != nullptr)
    {
        co_await this->server.log->flush();
    }
    co_return;
}

