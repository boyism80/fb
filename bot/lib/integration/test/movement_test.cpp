#include <fb/bot/integration/movement_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/game/protocol/object/direction.h>
#include <chrono>

using namespace std::chrono_literals;

namespace fb::bot::integration {

movement_test::movement_test(game_bot_controller& controller) :
    bot_integration_test(controller)
{
    // Register hook for object ID (sequence) responses
    this->_controller.hook_external(this, this, &movement_test::on_hook_sequence);
    fb::logger::debug("Movement test constructed");
}

async::task<void> movement_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 5;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Movement test initializing and spawning {} bots", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Movement test initialization completed - {} bots spawned", REQUIRED_BOTS);
    co_return;
}

async::task<void> movement_test::on_hook_sequence(fb::bot::game_bot&                      bot,
                                                  const fb::protocol::game::response::id& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Movement test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<bool> movement_test::execute()
{
    constexpr auto interval = 100ms;

    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto bots = this->get_test_bots();
    fb::logger::info("Starting movement test with {} bots", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for movement test");
        this->set_state(test_state::failed);
        co_return false;
    }

    // Use the last bot for movement
    auto target_bot = bots.back();

    fb::logger::info("Moving bot {} downward {} steps", target_bot->fd(), MOVEMENT_STEPS);

    // Perform movement steps
    for (auto i = 0; i < MOVEMENT_STEPS; i++)
    {
        auto position = target_bot->position();
        auto oid      = target_bot->oid();

        target_bot->send(fb::protocol::game::request::move{DIRECTION::BOTTOM, oid, position});

        position.y += 1;
        target_bot->set_position(position);

        fb::logger::debug("Movement step {}: bot {} moved to ({}, {})",
                          i + 1,
                          target_bot->fd(),
                          position.x,
                          position.y);

        auto thread = target_bot->thread();
        co_await thread->switching();
        co_await thread->sleep(interval);
    }

    this->set_state(test_state::completed);

    fb::logger::info("Movement test completed successfully");

    // Notify controller that this test is completed
    this->_controller.notify_test_completed(this);

    co_return true; // 성공
}

void movement_test::reset()
{
    this->_state = test_state::idle;

    fb::logger::info("Movement test reset");
}

bool movement_test::is_ready() const
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

void movement_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    bot_integration_test::on_bot_connected(bot);
    fb::logger::debug("Movement test: Bot {} added to collection", bot->fd());
}

} // namespace fb::bot::integration