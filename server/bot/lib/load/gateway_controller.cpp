#include <fb/bot/container.h>
#include <fb/bot/load/gateway_controller.h>
#include <fb/bot/gateway_bot.h>
#include <fb/bot/login_bot.h>
#include <fb/bot/login_controller.h>

using namespace fb::bot::load;

gateway_bot_controller::gateway_bot_controller(bot_container& container) :
    fb::bot::gateway_bot_controller(container),
    _remained_count(fb::config<uint32_t>("spawn_count") / fb::config<uint32_t>("io_size"))
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
}

async::task<void> gateway_bot_controller::on_bot_spawn()
{
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(fb::config<std::string>("ip")),
                                                   fb::config<uint16_t>("port"));
    auto count =
        std::min(this->_remained_count, fb::config<uint32_t>("spawn_per_interval") / fb::config<uint32_t>("io_size"));

    for (uint32_t i = 0; i < count; i++)
    {
        auto bot = this->create();
        bot->connect(endpoint);
    }

    this->_remained_count -= count;
    co_return;
}

async::task<void> gateway_bot_controller::on_welcome(gateway_bot& bot, const gateway_resp::welcome& response)
{
    bot.send(gateway_reqs::version{550, 0xD7}, false, true);
    co_return;
}

async::task<void> gateway_bot_controller::on_crt(gateway_bot& bot, const gateway_resp::encryption& response)
{
    bot.encryption(response.cryptor);
    bot.send(gateway_reqs::endpoint{0x01, 0});
    co_return;
}

async::task<void> gateway_bot_controller::on_hosts(gateway_bot& bot, const gateway_resp::server_list& response)
{
    bot.send(gateway_reqs::endpoint{0x00, 0});
    co_return;
}

async::task<void> gateway_bot_controller::on_transfer(gateway_bot& bot, const fb_resp::transfer& response)
{
    bot.close();

    auto created  = this->container.login->create(response.parameter);
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