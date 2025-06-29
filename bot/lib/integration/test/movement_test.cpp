#include <fb/bot/integration/movement_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/game/protocol/object/direction.h>
#include <chrono>

using namespace std::chrono_literals;

namespace fb::bot::integration {

async::task<void> movement_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 5;

    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(fb::config<std::string>("ip")),
                                                   fb::config<uint16_t>("port"));

    fb::logger::info("Movement test initializing and spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Movement test initialization completed - {} bots spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> movement_test::execute()
{
    if (this->_test_running || this->_test_completed)
        co_return false;

    this->_test_running = true;

    auto bots = this->get_test_bots();
    fb::logger::info("Starting movement test with {} bots", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for movement test");
        this->_test_running = false;
        co_return false;
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

    co_return true; // 성공
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

} // namespace fb::bot::integration