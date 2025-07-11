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

generator<bot_integration_test::scenario_t> movement_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->move_bot_downward();
    };
}

async::task<bool> movement_test::move_bot_downward()
{
    auto bots = this->get_test_bots();
    if (bots.empty())
        throw std::runtime_error("No bots available for movement test");

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
        co_await thread->sleep(DEFAULT_INTERVAL);
    }

    co_return true;
}

std::string movement_test::name() const
{
    return "Movement Test";
}

} // namespace fb::bot::integration