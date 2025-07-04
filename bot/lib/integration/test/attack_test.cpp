#include <fb/bot/integration/attack_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol/attack.h>
#include <chrono>

using namespace std::chrono_literals;

namespace fb::bot::integration {

async::task<void> attack_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 1;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Attack test initializing and spawning {} bot", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Attack test initialization completed - {} bot spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> attack_test::execute()
{
    constexpr auto interval = 250ms;

    if (this->_test_running || this->_test_completed)
        co_return false;

    this->_test_running = true;

    auto bots = this->get_test_bots();
    fb::logger::info("Starting attack test with {} bot", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for attack test");
        this->_test_running = false;
        co_return false;
    }

    // Use the first (and only) bot for attacks
    auto target_bot = bots.front();

    fb::logger::info("Bot {} performing {} attacks", target_bot->fd(), ATTACK_COUNT);

    // Perform attack sequences
    for (auto i = 0; i < ATTACK_COUNT; i++)
    {
        // Send attack packet
        target_bot->send(fb::protocol::game::request::attack{});

        fb::logger::debug("Attack {}: bot {} performed attack", i + 1, target_bot->fd());

        auto thread = target_bot->thread();
        co_await thread->switching();
        co_await thread->sleep(interval);
    }

    this->_test_completed = true;
    this->_test_running   = false;

    fb::logger::info("Attack test completed successfully - {} attacks performed", ATTACK_COUNT);

    // Cleanup bots after test completion
    this->cleanup();

    co_return true; // 성공
}

void attack_test::reset()
{
    this->_test_completed = false;
    this->_test_running   = false;

    fb::logger::info("Attack test reset");
}

bool attack_test::is_ready() const
{
    auto bots = this->get_test_bots();
    if (bots.empty())
        return false;

    // Movement test requires all bots to have non-zero oid
    for (const auto& bot : bots)
    {
        if (bot->oid() == 0)
            return false;
    }

    return true;
}

void attack_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    this->_test_bots.push_back(bot);
    fb::logger::debug("Attack test: Bot {} added to collection", bot->fd());
}

std::vector<std::shared_ptr<fb::bot::game_bot>> attack_test::get_test_bots() const
{
    return this->_test_bots;
}

void attack_test::cleanup()
{
    for (auto bot : this->_test_bots)
    {
        if (bot)
        {
            bot->close();
        }
    }
    this->_test_bots.clear();

    fb::logger::info("Attack test cleanup completed - all bots disconnected");
}

} // namespace fb::bot::integration