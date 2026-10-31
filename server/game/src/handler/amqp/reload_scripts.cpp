#include <fb/game/handler/amqp/reload_scripts.h>

#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/model/script_download.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
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
            auto backups = std::vector<std::pair<std::filesystem::path, std::optional<std::string>>>{};
            if (message.url.empty() == false)
            {
                for (const auto& relative : message.script_paths)
                {
                    auto path = std::filesystem::path("scripts") /
                                std::filesystem::path(fb::model::normalize_script_relative_path(relative));
                    auto in = std::ifstream(path, std::ios::binary);
                    if (in.is_open())
                        backups.emplace_back(path, std::string(std::istreambuf_iterator<char>(in), {}));
                    else
                        backups.emplace_back(path, std::nullopt);
                }

                co_await fb::model::download_scripts(this->server.http, message.url, message.script_paths);
            }

            try
            {
                co_await this->server.lua.validate_scripts(message.script_paths);
            }
            catch (std::exception&)
            {
                for (const auto& [path, bytes] : backups)
                {
                    if (bytes.has_value())
                    {
                        auto out = std::ofstream(path, std::ios::binary | std::ios::trunc);
                        out.write(bytes->data(), static_cast<std::streamsize>(bytes->size()));
                        if (!out)
                            fb::logger::fatal("Script restore failed: {}", path.string());
                    }
                    else
                    {
                        std::error_code ec;
                        std::filesystem::remove(path, ec);
                    }
                }
                throw;
            }

            co_await this->server.lua.reload_scripts(message.script_paths);
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
