#include <fb/bot/load/game_controller.h>

#include <async/awaitable_get.h>
#include <fb/bot/container.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/load/gateway_controller.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/model/loader.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

using namespace fb::bot::load;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container)
{ }

void game_bot_controller::initialize()
{
    table::item->hook.build = &game_bot_controller::build_item;
    async::awaitable_get(fb::model::loader(this->container).run());

    this->bind_thread_timer(&game_bot_controller::on_timer, 100ms);
    this->bind_timer(&game_bot_controller::on_metrics,
                     std::chrono::milliseconds(fb::config<uint32_t>("load:metrics_interval_ms", 10000)));
}

async::task<void> game_bot_controller::on_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->container.threads.at(id);
    auto params = thread->data<bot_thread_params>();
    if (params == nullptr)
        co_return;

    auto guard = this->_bots.enter_read();
    for (auto& [_, bot] : params->bots)
    {
        if (guard.value().contains(bot->id) == false)
            continue;

        auto typed_bot = std::static_pointer_cast<game_bot>(bot);
        if (typed_bot->inited() == false && typed_bot->oid() != 0 && typed_bot->map() != 0xFFFF &&
            typed_bot->has_position() && typed_bot->name().empty() == false)
        {
            typed_bot->inited(true);
            this->_online++;
            fb::logger::info("load bot: bot={} behavior not implemented", typed_bot->name());
        }
    }
    co_return;
}

async::task<void> game_bot_controller::on_metrics()
{
    auto root      = Json::Value{};
    root["time"]   = std::format("{}", std::chrono::system_clock::now());
    root["online"] = this->_online.load();

    auto writer           = Json::StreamWriterBuilder{};
    writer["indentation"] = "";
    auto line             = Json::writeString(writer, root);

    std::filesystem::create_directories("logs");
    auto file = std::ofstream(std::format("logs/load-metrics-{}.jsonl", fb::config<uint32_t>("load:process", 0)),
                              std::ios::app);
    file << line << std::endl;
    fb::logger::info("load metrics: {}", line);

    root["type"] = "metrics";
    this->container.link.send(root);
    co_return;
}

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    // Send game login packet with transfer buffer.
    // Bootstrap 0x10 always uses the v550 layout; the packed client_version field
    // inside the transfer blob still establishes the session version.
    using login_request = fb::protocol::game::request::login<fb::protocol::CLIENT_VERSION::v550>;
    bot.send(login_request(bot.transfer_buffer()), false, true);

    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    if (bot.inited())
        this->_online--;
    if (bot.credential().has_value())
        std::static_pointer_cast<gateway_bot_controller>(this->container.gateway)
            ->relogin(bot, bot.credential().value());
    co_await super::on_bot_disconnected(bot);
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}
