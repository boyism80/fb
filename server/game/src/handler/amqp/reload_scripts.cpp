#include <fb/game/handler/amqp/reload_scripts.h>

#include <fb/config.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/model/script_download.h>

#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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
        if (message.script_paths.empty() == false)
        {
            auto files = std::vector<fb::lua::context_pool::script_file>{};
            files.reserve(message.script_paths.size());
            if (message.url.empty() == false)
            {
                auto requested = message.url;
                auto allowed   = fb::config<std::string>("download:script", std::string());
                while (!requested.empty() && requested.back() == '/')
                {
                    requested.pop_back();
                }

                while (!allowed.empty() && allowed.back() == '/')
                {
                    allowed.pop_back();
                }

                if (allowed.empty() || requested != allowed)
                    throw std::runtime_error(std::format("script download url is not allowed: {}", message.url));

                for (const auto& name : message.script_paths)
                {
                    auto relative = fb::model::normalize_script_relative_path(name);
                    auto url      = fb::model::join_script_download_url(message.url, relative);
                    auto bytes    = co_await this->server.http.get_bytes(url);
                    if (bytes.empty())
                        throw std::runtime_error(std::format("downloaded empty body: {}", url));

                    files.push_back({std::move(relative), std::move(bytes)});
                }
            }
            else
            {
                for (const auto& name : message.script_paths)
                {
                    auto relative = fb::model::normalize_script_relative_path(name);
                    auto in       = std::ifstream(std::filesystem::path("scripts") / relative, std::ios::binary);
                    if (in.is_open() == false)
                        throw std::runtime_error(std::format("script not found: {}", relative));

                    auto bytes = std::vector<uint8_t>(std::istreambuf_iterator<char>(in), {});
                    files.push_back({std::move(relative), std::move(bytes)});
                }
            }

            co_await this->server.lua.patch_scripts(files);
        }

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
