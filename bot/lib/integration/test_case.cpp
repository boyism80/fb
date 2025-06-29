#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/game/protocol/object/direction.h>
#include <fb/bot/integration/gateway_controller.h>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace fb::bot::integration {

// Base test class implementation
void bot_integration_test::cleanup()
{
    fb::logger::info("Test cleanup completed (base implementation)");
}

// Movement Test Implementation
async::task<void> movement_test::spawn_bots(game_bot_controller&                  controller,
                                            const boost::asio::ip::tcp::endpoint& endpoint)
{
    constexpr auto REQUIRED_BOTS = 5;

    fb::logger::info("Movement test spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Movement test bots spawned successfully");
    co_return;
}

async::task<void> movement_test::execute()
{
    if (this->_test_running || this->_test_completed)
        co_return;

    this->_test_running = true;

    auto bots = this->get_test_bots();
    fb::logger::info("Starting movement test with {} bots", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for movement test");
        this->_test_running = false;
        co_return;
    }

    // Use the last bot for movement
    auto target_bot = bots.back();

    fb::logger::info("Moving bot {} downward {} steps", target_bot->fd(), MOVEMENT_STEPS);

    // Perform movement steps
    for (auto i = 0; i < MOVEMENT_STEPS; i++)
    {
        auto position = target_bot->position();
        auto sequence = target_bot->sequence();

        target_bot->send(fb::protocol::game::request::move{DIRECTION::BOTTOM, sequence, position});

        position.y += 1;
        target_bot->set_position(position);

        fb::logger::debug("Movement step {}: bot {} moved to ({}, {})",
                          i + 1,
                          target_bot->fd(),
                          position.x,
                          position.y);

        auto thread = target_bot->thread();
        co_await thread->sleep(1s);
    }

    this->_test_completed = true;
    this->_test_running   = false;

    fb::logger::info("Movement test completed successfully");

    // Cleanup bots after test completion
    this->cleanup();
}

void movement_test::reset()
{
    this->_test_started   = false;
    this->_test_completed = false;
    this->_test_running   = false;

    fb::logger::info("Movement test reset");
}

bool movement_test::is_ready() const
{
    auto bots = this->get_test_bots();

    if (bots.empty())
        return false;

    // Movement test requires all bots to have non-zero sequence
    for (const auto& bot : bots)
    {
        if (bot->sequence() == 0)
            return false;
    }

    return true;
}

void movement_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    this->_test_bots.push_back(bot);
    fb::logger::debug("Movement test: Bot {} added to collection", bot->fd());
}

std::vector<std::shared_ptr<fb::bot::game_bot>> movement_test::get_test_bots() const
{
    return this->_test_bots;
}

void movement_test::cleanup()
{
    for (auto bot : this->_test_bots)
    {
        if (bot)
        {
            bot->close();
        }
    }
    this->_test_bots.clear();

    fb::logger::info("Movement test cleanup completed - all bots disconnected");
}

// Attack Test Implementation
async::task<void> attack_test::spawn_bots(game_bot_controller&                  controller,
                                          const boost::asio::ip::tcp::endpoint& endpoint)
{
    constexpr auto REQUIRED_BOTS = 3;

    fb::logger::info("Attack test spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Attack test bots spawned successfully");
    co_return;
}

async::task<void> attack_test::execute()
{
    if (this->_test_running || this->_test_completed)
        co_return;

    this->_test_running = true;

    fb::logger::info("Starting attack test");

    // TODO: Implement attack test logic
    std::this_thread::sleep_for(2s);
    co_await std::suspend_always{};

    this->_test_completed = true;
    this->_test_running   = false;

    fb::logger::info("Attack test completed (placeholder implementation)");

    // Cleanup bots after test completion
    this->cleanup();
}

void attack_test::reset()
{
    this->_test_completed = false;
    this->_test_running   = false;

    fb::logger::info("Attack test reset");
}

bool attack_test::is_ready() const
{
    // Attack test doesn't manage individual bots yet
    // TODO: Define specific readiness criteria for attack test
    return true; // Always ready for now
}

void attack_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    // Attack test doesn't store individual bots yet
    fb::logger::debug("Attack test: Bot {} connected (not stored)", bot->fd());
}

// Skill Test Implementation
async::task<void> skill_test::spawn_bots(game_bot_controller&                  controller,
                                         const boost::asio::ip::tcp::endpoint& endpoint)
{
    constexpr auto REQUIRED_BOTS = 2;

    fb::logger::info("Skill test spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Skill test bots spawned successfully");
    co_return;
}

async::task<void> skill_test::execute()
{
    if (this->_test_running || this->_test_completed)
        co_return;

    this->_test_running = true;

    fb::logger::info("Starting skill test");

    // TODO: Implement skill test logic
    std::this_thread::sleep_for(3s);
    co_await std::suspend_always{};

    this->_test_completed = true;
    this->_test_running   = false;

    fb::logger::info("Skill test completed (placeholder implementation)");

    // Cleanup bots after test completion
    this->cleanup();
}

void skill_test::reset()
{
    this->_test_completed = false;
    this->_test_running   = false;

    fb::logger::info("Skill test reset");
}

bool skill_test::is_ready() const
{
    // Skill test doesn't manage individual bots yet
    // TODO: Define specific readiness criteria for skill test
    return true; // Always ready for now
}

void skill_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    // Skill test doesn't store individual bots yet
    fb::logger::debug("Skill test: Bot {} connected (not stored)", bot->fd());
}

} // namespace fb::bot::integration