#include <fb/game/handler/amqp/reload_tables.h>
#include <fb/game/server.h>
#include <fb/model/loader.h>
#include <fb/model/table_download.h>
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
        if (message.url.empty() == false && message.table_names.empty() == false)
            co_await fb::model::download_tables(this->server.http, message.url, message.table_names);

        // Build+publish runs on a background thread; this logic thread only awaits.
        co_await fb::model::reload_async();
        fb::logger::info("Data tables reloaded");

        this->server.characters.foreach_enqueue(
            [](auto& ch) -> async::task<void> {
                ch->message("테이블 패치가 완료되었습니다.", MESSAGE_TYPE::NOTIFY);
                co_return;
            },
            [](const auto& ch) {
                return ch != nullptr && ch->role() >= ROLE::ADMIN;
            });
    }
    catch (std::exception& e)
    {
        fb::logger::fatal("Data table reload failed: {}", e.what());
    }
    co_return;
}
