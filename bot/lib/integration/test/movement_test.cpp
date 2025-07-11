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
    bot_integration_test(controller, 5) // Spawn 5 bots
{
    fb::logger::debug("Movement test constructed");
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
    this->notify_completed();

    co_return true; // 성공
}

void movement_test::reset()
{
    bot_integration_test::reset();
    fb::logger::info("Movement test reset");
}

std::string movement_test::name() const
{
    return "Movement Test";
}

} // namespace fb::bot::integration