#include <fb/bot/bot.login.controller.h>
#include <fb/bot/bot.game.h>
#include <fb/bot/bot.login.h>
#include <fb/bot/bot.game.controller.h>

using namespace fb::bot;

login_bot_controller::login_bot_controller(bot_container& container) :
    bot_controller<login_bot>(container)
{
    this->bind(&login_bot_controller::handle_agreement);
    this->bind(&login_bot_controller::handle_transfer);
}

async::task<void> login_bot_controller::handle_agreement(login_bot&                                      bot,
                                                         const fb::protocol::login::response::agreement& response)
{
    auto           id     = bot.generate_id();
    auto           exists = false;
    constexpr auto pw     = "admin123";

    try
    {
        auto thread = bot.thread();

        while (true)
        {
            auto&& resp = co_await bot.request<fb::protocol::login::response::message>(
                fb::protocol::login::request::create(id, pw));

            if (resp.type == 0x00)
                break;

            if (resp.type == 0x0E)
            {
                if (resp.text == fb::model::const_value::string::MESSAGE_ACCOUNT_ALREADY_EXISTS)
                {
                    exists = true;
                    break;
                }
                else
                {
                    id = bot.generate_id();
                }
            }

            co_await thread->sleep(100ms);
        }

        if (exists == false)
        {
            std::random_device rd;
            std::mt19937       gen(rd());

            uint8_t hair     = std::uniform_int_distribution<>(0, 101)(gen);
            uint8_t sex      = std::uniform_int_distribution<>(0, 1)(gen);
            uint8_t nation   = std::uniform_int_distribution<>(0, 1)(gen);
            uint8_t creature = std::uniform_int_distribution<>(0, 3)(gen);

            while (true)
            {
                auto&& resp = co_await bot.request<fb::protocol::login::response::message>(
                    fb::protocol::login::request::complete{hair, sex, nation, creature});

                if (resp.type == 0x00)
                    break;

                co_await thread->sleep(100ms);
            }
        }

        while (true)
        {
            auto&& resp = co_await bot.request<fb::protocol::login::response::message>(
                fb::protocol::login::request::login{id, pw});
            if (resp.type == 0x00)
                break;

            co_await thread->sleep(1000ms);
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    co_return;
}

async::task<void> login_bot_controller::handle_transfer(login_bot&                              bot,
                                                        const fb::protocol::response::transfer& response)
{
    bot.close();

    auto created  = this->container.game_controller->create(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> login_bot_controller::on_bot_connected(login_bot& bot)
{
    auto& crypto = bot.crt();
    bot.send(fb::protocol::login::request::agreement(crypto.type(), crypto.KEY_SIZE, crypto.key()), false, true);

    // Bot is now managed by controller's thread-safe collection
    co_return;
}

async::task<void> login_bot_controller::on_bot_disconnected(login_bot& bot)
{
    // Bot is automatically removed from controller's thread-safe collection
    co_return;
}

bool login_bot_controller::decrypt_policy(int cmd) const
{
    switch (cmd)
    {
    case fb::protocol::response::transfer::header: // Host discovery
        return false;

    default:
        return true;
    }
}