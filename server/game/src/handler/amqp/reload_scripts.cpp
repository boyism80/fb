#include <fb/game/handler/amqp/reload_scripts.h>
#include <fb/game/server.h>
#include <fb/model/script_download.h>
#include <fb/logger.h>

using namespace fb::game::handler::amqp;

reload_scripts::reload_scripts(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::ReloadScripts>(server)
{ }

async::task<void> reload_scripts::handle(const internal_resp::ReloadScripts& message)
{
    if (message.error != 0)
        co_return;

    try
    {
        if (message.url.empty() == false && message.script_paths.empty() == false)
            co_await fb::model::download_scripts(this->server.http, message.url, message.script_paths);

        if (message.script_paths.empty() == false)
            co_await this->server.lua.reload_scripts(message.script_paths);

        fb::logger::info("Scripts reloaded ({})", message.script_paths.size());

        this->server.characters.foreach_enqueue(
            [](auto& ch) -> async::task<void> {
                ch->message("스크립트 패치가 완료되었습니다.", MESSAGE_TYPE::NOTIFY);
                co_return;
            },
            [](const auto& ch) {
                return ch != nullptr && ch->role() >= ROLE::ADMIN;
            });
    }
    catch (std::exception& e)
    {
        fb::logger::fatal("Script reload failed: {}", e.what());
    }
    co_return;
}
