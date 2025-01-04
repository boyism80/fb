#include <fb/bot/bot.login.h>
#include <fb/bot/bot.gateway.h>
#include <fb/bot/bot.container.h>

using namespace fb::bot;

gateway_bot::gateway_bot(bot_container& owner, uint32_t id) :
    base_bot(owner, id)
{
    this->bind(&gateway_bot::handle_welcome);
    this->bind(&gateway_bot::handle_crt);
    this->bind(&gateway_bot::handle_hosts);
    this->bind(&gateway_bot::handle_transfer);
}

gateway_bot::~gateway_bot()
{ }

async::task<void> gateway_bot::on_connected()
{
    {
        auto _ = std::lock_guard<std::shared_mutex>(_mutex);
        _count++;
    }

    co_await base_bot::on_connected();
}

async::task<void> gateway_bot::on_disconnected()
{
    {
        auto _ = std::lock_guard<std::shared_mutex>(_mutex);
        _count--;
    }

    co_await base_bot::on_disconnected();
}

async::task<void> gateway_bot::handle_welcome(const fb::protocol::gateway::response::welcome& response)
{
    this->send(fb::protocol::gateway::request::version{550, 0xD7}, false, true);
    co_return;
}

async::task<void> gateway_bot::handle_crt(const fb::protocol::gateway::response::crypto& response)
{
    this->_crypto = response.crt;
    this->send(fb::protocol::gateway::request::endpoint{0x01, 0});
    co_return;
}

async::task<void> gateway_bot::handle_hosts(const fb::protocol::gateway::response::endpoint& response)
{
    this->send(fb::protocol::gateway::request::endpoint{0x00, 1});
    co_return;
}

async::task<void> gateway_bot::handle_transfer(const fb::protocol::response::transfer& response)
{
    this->close();

    auto bot      = this->_owner.create<login_bot>(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    bot->connect(endpoint);
    co_return;
}

bool gateway_bot::decrypt_policy(int cmd) const
{
    switch (cmd)
    {
    case 0x00:
    case 0x7E:
        return false;

    default:
        return base_bot::decrypt_policy(cmd);
    }
}