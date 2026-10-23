#include <fb/bot/integration/login_controller.h>

#include <fb/bot/game_controller.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/login_bot.h>
#include <fb/logger.h>

#include <chrono>
#include <cstdint>
#include <exception>
#include <format>
#include <random>
#include <stdexcept>
#include <string>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace {

#ifdef _DEBUG
constexpr auto LOGIN_REQUEST_TIMEOUT = 30s;
#else
constexpr auto LOGIN_REQUEST_TIMEOUT = 10s;
#endif

constexpr auto RELOGIN_TIMEOUT = 30s;

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
    auto relogin = bot.credential().has_value();
    auto id      = relogin ? bot.credential()->id : bot.generate_id();
    auto pw      = relogin ? bot.credential()->pw : std::string{"admin123"};

    try
    {
        auto thread = bot.thread();

        fb::logger::debug("login flow start: bot_id={} account={} relogin={}", bot.id, id, relogin);

        while (relogin == false)
        {
            fb::logger::debug("login create request: bot_id={} account={}", bot.id, id);
            auto&& resp =
                co_await bot.request<login_resp::message>(*bot.make<login_reqs::create>(id, pw), LOGIN_REQUEST_TIMEOUT);

            if (resp.type == 0x00)
                break;

            if (resp.type == 0x0E)
            {
                fb::logger::debug("login create rejected: bot_id={} account={} text={}", bot.id, id, resp.text);
                id = bot.generate_id();
            }

            co_await thread->sleep(100ms);
        }

        if (relogin == false)
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
                    *bot.make<login_reqs::complete>(hair, gender, nation, divine_beast),
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

        // A relogin right after logout is refused as already logged in until the previous game server has saved
        // and released the session, so it keeps retrying like a real client would.
        fb::logger::debug("login auth request: bot_id={} account={}", bot.id, id);
        auto relogin_deadline = std::chrono::steady_clock::now() + RELOGIN_TIMEOUT;
        while (true)
        {
            auto&& resp =
                co_await bot.request<login_resp::message>(*bot.make<login_reqs::login>(id, pw), LOGIN_REQUEST_TIMEOUT);
            if (resp.type == 0x00)
                break;

            fb::logger::debug("login auth rejected: bot_id={} type=0x{:02X} text={}", bot.id, resp.type, resp.text);
            if (relogin && std::chrono::steady_clock::now() >= relogin_deadline)
                throw std::runtime_error(std::format("relogin rejected until timeout: {}", resp.text));

            co_await thread->sleep(1000ms);
        }

        bot.credential(fb::bot::credential{id, pw});
        fb::logger::debug("login flow done: bot_id={} account={}", bot.id, id);
    }
    catch (std::exception& e)
    {
        fb::logger::fatal("login flow failed: bot_id={} account={} error={}", bot.id, id, e.what());
        bot.close();
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
    created->carry_over(bot);
    created->credential(bot.credential());
    if (bot.reconnect_from() != 0)
        created->set_transfer_from_bot_id(bot.reconnect_from());
    if (auto* game = dynamic_cast<fb::bot::integration::game_bot_controller*>(this->container.game.get()))
        game->move_owner(bot.id, created->id);

    created->connect(endpoint);

    // TODO: Validate seamless transition to game server
    co_return;
}

async::task<void> login_bot_controller::on_bot_connected(login_bot& bot)
{
    // Integration test: Initialize authentication test scenario upon connection
    auto& encryption = bot.encryption();
    bot.send(*bot.make<login_reqs::agreement>(encryption.pattern(),
                                              fb::encryption::KEY_SIZE,
                                              encryption.iv(),
                                              bot.transfer_from(),
                                              bot.client_version(),
                                              bot.ui_mode()),
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