#include <fb/bot/load/gateway_controller.h>

#include <fb/bot/container.h>
#include <fb/bot/game_controller.h>
#include <fb/bot/gateway_bot.h>
#include <fb/bot/login_bot.h>
#include <fb/bot/login_controller.h>
#include <fb/protocol/client_version.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace fb::bot::load;

gateway_bot_controller::gateway_bot_controller(bot_container& container) :
    fb::bot::gateway_bot_controller(container),
    _target(fb::config<uint32_t>("spawn_count"))
{
    this->bind(&gateway_bot_controller::on_welcome);
    this->bind(&gateway_bot_controller::on_crt);
    this->bind(&gateway_bot_controller::on_hosts);
    this->bind(&gateway_bot_controller::on_transfer);
}

void gateway_bot_controller::initialize()
{
    this->bind_timer(&gateway_bot_controller::on_bot_spawn,
                     std::chrono::milliseconds(fb::config<uint32_t>("interval")));

    this->container.link.on("scale", [this](const Json::Value& message) {
        auto target = message["users"].asUInt();
        boost::asio::post(this->container.io_context, [this, target]() {
            this->scale(target);
        });
    });
}

void gateway_bot_controller::scale(uint32_t target)
{
    this->_target = target;
    if (this->_spawned <= target)
        return;

    auto bots    = std::vector<std::shared_ptr<fb::bot::base_bot>>{};
    auto collect = [&bots](const auto& entries) {
        for (auto& [_, bot] : entries)
        {
            bots.push_back(bot);
        }
    };
    this->read_bots(collect);
    this->container.login->read_bots(collect);
    this->container.game->read_bots(collect);
    std::sort(bots.begin(), bots.end(), [](const auto& left, const auto& right) {
        return left->id > right->id;
    });

    for (size_t i = 0; i + target < bots.size(); i++)
    {
        bots.at(i)->close();
    }
    this->_spawned = target;
}

async::task<void> gateway_bot_controller::on_bot_spawn()
{
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(fb::config<std::string>("ip")),
                                                   fb::config<uint16_t>("port"));
    auto spawned  = this->_spawned.load();
    auto target   = this->_target.load();
    auto count    = target > spawned ? std::min(target - spawned, fb::config<uint32_t>("spawn_per_interval")) : 0u;

    for (uint32_t i = 0; i < count; i++)
    {
        auto bot = this->create();
        bot->profile(fb::bot::client_profile::pick());
        bot->connect(endpoint);
    }

    this->_spawned += count;
    co_return;
}

async::task<void> gateway_bot_controller::on_welcome(gateway_bot& bot, const gateway_resp::welcome& response)
{
    auto nation = static_cast<uint8_t>(fb::config<uint16_t>("client:nation", 0xD7));
    bot.send(*bot.make<gateway_reqs::version>(bot.client_version(), nation), false, true);
    co_return;
}

async::task<void> gateway_bot_controller::on_crt(gateway_bot& bot, const gateway_resp::encryption& response)
{
    bot.encryption(response.cryptor);
    bot.send(*bot.make<gateway_reqs::server_list>(uint8_t{0x01}, uint8_t{0}));
    co_return;
}

async::task<void> gateway_bot_controller::on_hosts(gateway_bot& bot, const gateway_resp::server_list& response)
{
    bot.send(*bot.make<gateway_reqs::server_list>(uint8_t{0x00}, uint8_t{0}));
    co_return;
}

async::task<void> gateway_bot_controller::on_transfer(gateway_bot& bot, const fb_resp::transfer& response)
{
    bot.close();

    auto created = this->container.login->create(response.parameter);
    created->carry_over(bot);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> gateway_bot_controller::on_bot_connected(gateway_bot& bot)
{
    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> gateway_bot_controller::on_bot_disconnected(gateway_bot& bot)
{
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}