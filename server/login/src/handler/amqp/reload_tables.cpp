#include <fb/login/handler/amqp/reload_tables.h>
#include <fb/login/server.h>
#include <fb/model/loader.h>
#include <fb/model/table_download.h>
#include <fb/logger.h>

using namespace fb::login::handler::amqp;

reload_tables::reload_tables(fb::login::server& server) :
    fb::handler::amqp<fb::login::server, internal_resp::ReloadTables>(server)
{ }

async::task<void> reload_tables::handle(const internal_resp::ReloadTables& message)
{
    if (message.error != 0)
        co_return;

    try
    {
        if (message.url.empty() == false && message.table_names.empty() == false)
            co_await fb::model::download_tables(this->server.http, message.url, message.table_names);

        co_await fb::model::reload_async();
        fb::logger::info("Data tables reloaded");
    }
    catch (std::exception& e)
    {
        fb::logger::fatal("Data table reload failed: {}", e.what());
    }
    co_return;
}
