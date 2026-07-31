#include <fb/game/handler/amqp/reload_tables.h>
#include <fb/game/server.h>
#include <fb/model/loader.h>
#include <fb/logger.h>

using namespace fb::game::handler::amqp;

reload_tables::reload_tables(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::ReloadTables>(server)
{ }

async::task<void> reload_tables::handle(const internal_resp::ReloadTables& message)
{
    if (message.error != 0)
        co_return;

    try
    {
        // Build+publish runs on a background thread; this logic thread only awaits.
        co_await fb::model::reload_async();
        fb::logger::info("Data tables reloaded");
    }
    catch (std::exception& e)
    {
        fb::logger::fatal("Data table reload failed: {}", e.what());
    }
    co_return;
}
