#include <fb/game/handler/amqp/storage_pending_fetch.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

storage_pending_fetch::storage_pending_fetch(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::GetStoragePending>(server)
{
}

async::task<void> storage_pending_fetch::handle(const internal_resp::GetStoragePending&)
{
    co_await this->server.fetch_storage_pending();
    co_return;
}

