#include <fb/bot/bot.h>

using namespace fb::bot;

gateway_bot::gateway_bot(bot_container& owner) : base_bot(owner)
{
    this->bind<fb::protocol::gateway::response::welcome>(std::bind(&gateway_bot::handle_welcome, this, std::placeholders::_1));
    this->bind<fb::protocol::gateway::response::crt>(std::bind(&gateway_bot::handle_crt, this, std::placeholders::_1));
    this->bind<fb::protocol::gateway::response::hosts>(std::bind(&gateway_bot::handle_hosts, this, std::placeholders::_1));
    this->bind<fb::protocol::response::transfer>(std::bind(&gateway_bot::handle_transfer, this, std::placeholders::_1));
}

gateway_bot::~gateway_bot()
{}

void gateway_bot::handle_welcome(const fb::protocol::gateway::response::welcome& response)
{
    this->send(fb::protocol::gateway::request::assert_version { 0x0226, 0xD7 }, false, true);
}

void gateway_bot::handle_crt(const fb::protocol::gateway::response::crt& response)
{
    this->_cryptor = response.cryptor;
    this->send(fb::protocol::gateway::request::entry_list { 0x01, 0 });
}

void gateway_bot::handle_hosts(const fb::protocol::gateway::response::hosts& response)
{
    this->send(fb::protocol::gateway::request::entry_list { 0x00, 1 });
}

void gateway_bot::handle_transfer(const fb::protocol::response::transfer& response)
{
    auto bot = this->_owner.create<login_bot>(response.parameter);
    auto ip = boost::asio::ip::address_v4(_byteswap_ulong(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    bot->connect(endpoint);

    this->close();
}

bool gateway_bot::is_decrypt(int cmd) const
{
    switch(cmd)
    {
        case 0x00:
        case 0x7E:
        return false;

        default:
        return base_bot::is_decrypt(cmd);
    }
}