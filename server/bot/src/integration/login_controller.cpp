#include <fb/bot/integration/login_controller.h>
#include <fb/bot/login_bot.h>
#include <fb/bot/game_controller.h>
#include <fb/logger.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace {

#ifdef _DEBUG
constexpr auto LOGIN_REQUEST_TIMEOUT = 30s;
#else
constexpr auto LOGIN_REQUEST_TIMEOUT = 10s;
#endif

} // namespace

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

async::task<void> login_bot_controller::on_agreement(login_bot& bot, const login_resp::terms_agreement& response)
{
    // Integration test: Validate authentication flow with controlled test accounts
    auto           id = bot.generate_id();
    constexpr auto pw = "admin123";

    try
    {
        auto thread = bot.thread();

        fb::logger::debug("login flow start: bot_id={} account={}", bot.id, id);

        while (true)
        {
            fb::logger::debug("login create request: bot_id={} account={}", bot.id, id);
            auto&& resp = co_await bot.request<login_resp::message>(fb::protocol::login::request::create(id, pw),
                                                                    LOGIN_REQUEST_TIMEOUT);

            if (resp.type == 0x00)
                break;

            if (resp.type == 0x0E)
            {
                fb::logger::debug("login create rejected: bot_id={} account={} text={}", bot.id, id, resp.text);
                id = bot.generate_id();
            }

            co_await thread->sleep(100ms);
        }

        {
            // Integration test: Validate account creation with specific test parameters
            std::random_device rd;
            std::mt19937       gen(rd());

            uint8_t hair         = std::uniform_int_distribution<>(0, 101)(gen);
            uint8_t gender       = std::uniform_int_distribution<>(0, 1)(gen);
            uint8_t nation       = std::uniform_int_distribution<>(1, 2)(gen);
            uint8_t divine_beast = std::uniform_int_distribution<>(0, 3)(gen);

            fb::logger::debug("login complete request: bot_id={} account={}", bot.id, id);
            while (true)
            {
                auto&& resp = co_await bot.request<login_resp::message>(
                    fb::protocol::login::request::complete{hair, gender, nation, divine_beast},
                    LOGIN_REQUEST_TIMEOUT);

                if (resp.type == 0x00)
                    break;

                fb::logger::debug("login complete rejected: bot_id={} type=0x{:02X} text={}",
                                  bot.id,
                                  resp.type,
                                  resp.text);
                co_await thread->sleep(100ms);
            }

            // TODO: Validate character creation parameters
        }

        fb::logger::debug("login auth request: bot_id={} account={}", bot.id, id);
        while (true)
        {
            auto&& resp = co_await bot.request<login_resp::message>(fb::protocol::login::request::login{id, pw},
                                                                    LOGIN_REQUEST_TIMEOUT);
            if (resp.type == 0x00)
                break;

            fb::logger::debug("login auth rejected: bot_id={} type=0x{:02X} text={}", bot.id, resp.type, resp.text);
            co_await thread->sleep(1000ms);
        }

        fb::logger::debug("login flow done: bot_id={} account={}", bot.id, id);
    }
    catch (std::exception& e)
    {
        fb::logger::fatal("login flow failed: bot_id={} account={} error={}", bot.id, id, e.what());
    }

    co_return;
}

async::task<void> login_bot_controller::on_transfer(login_bot& bot, const fb_resp::transfer& response)
{
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    fb::logger::debug("login transfer: bot_id={} game={}:{}", bot.id, ip.to_string(), response.port);

    bot.close();

    auto created = this->container.game->create(response.parameter);
    created->connect(endpoint);

    // TODO: Validate seamless transition to game server
    co_return;
}

async::task<void> login_bot_controller::on_bot_connected(login_bot& bot)
{
    // Integration test: Initialize authentication test scenario upon connection
    auto& encryption = bot.encryption();
    bot.send(login_reqs::agreement(encryption.pattern(),
                                   fb::encryption::KEY_SIZE,
                                   encryption.iv(),
                                   bot.transfer_from(),
                                   bot.client_version()),
             false,
             true);

    // TODO: Set up login-specific test scenarios
    co_return;
}

async::task<void> login_bot_controller::on_bot_disconnected(login_bot& bot)
{
    // Integration test: Collect test results and perform cleanup
    // TODO: Generate test report for this login bot session
    co_return;
}