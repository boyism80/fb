#include <fb/bot/integration/login_controller.h>
#include <fb/bot/login_bot.h>
#include <fb/bot/game_controller.h>

using namespace fb::bot::integration;

login_bot_controller::login_bot_controller(bot_container& container) :
    fb::bot::login_bot_controller(container)
{
    // Bind integration test specific handlers
    this->bind(&login_bot_controller::on_agreement);
    this->bind(&login_bot_controller::on_transfer);
}

void login_bot_controller::initialize()
{
    // No timers needed for login bot_controller as it's reactive to connections
}

async::task<void> login_bot_controller::on_agreement(login_bot& bot, const login_resp::agreement& response)
{
    // Integration test: Validate authentication flow with controlled test accounts
    auto           id     = bot.generate_id();
    auto           exists = false;
    constexpr auto pw     = "admin123";

    try
    {
        auto thread = bot.thread();

        // TODO: Add authentication flow validation logic
        while (true)
        {
            auto&& resp = co_await bot.request<login_resp::message>(fb::protocol::login::request::create(id, pw));

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
            // Integration test: Validate account creation with specific test parameters
            std::random_device rd;
            std::mt19937       gen(rd());

            uint8_t hair     = std::uniform_int_distribution<>(0, 101)(gen);
            uint8_t gender   = std::uniform_int_distribution<>(0, 1)(gen);
            uint8_t nation   = std::uniform_int_distribution<>(0, 1)(gen);
            uint8_t creature = std::uniform_int_distribution<>(0, 3)(gen);

            while (true)
            {
                auto&& resp = co_await bot.request<login_resp::message>(
                    fb::protocol::login::request::complete{hair, gender, nation, creature});

                if (resp.type == 0x00)
                    break;

                co_await thread->sleep(100ms);
            }

            // TODO: Validate character creation parameters
        }

        // Integration test: Validate login authentication
        while (true)
        {
            auto&& resp = co_await bot.request<login_resp::message>(fb::protocol::login::request::login{id, pw});
            if (resp.type == 0x00)
                break;

            co_await thread->sleep(1000ms);
        }

        // TODO: Validate successful authentication and session establishment
    }
    catch (std::exception& e)
    {
        // TODO: Log authentication errors for test analysis
        std::cout << e.what() << std::endl;
    }

    co_return;
}

async::task<void> login_bot_controller::on_transfer(login_bot& bot, const fb_resp::transfer& response)
{
    // Integration test: Validate login-to-game server transition
    bot.close();

    // TODO: Add transfer validation and test continuation logic
    auto created  = this->container.game->create(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    // TODO: Validate seamless transition to game server
    co_return;
}

async::task<void> login_bot_controller::on_bot_connected(login_bot& bot)
{
    // Integration test: Initialize authentication test scenario upon connection
    auto& encryption = bot.encryption();
    bot.send(login_reqs::agreement(encryption.pattern(), fb::encryption::KEY_SIZE, encryption.iv()), false, true);

    // TODO: Set up login-specific test scenarios
    co_return;
}

async::task<void> login_bot_controller::on_bot_disconnected(login_bot& bot)
{
    // Integration test: Collect test results and perform cleanup
    // TODO: Generate test report for this login bot session
    co_return;
}