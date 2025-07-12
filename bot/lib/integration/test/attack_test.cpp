#include <fb/bot/integration/attack_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <chrono>

using namespace std::chrono_literals;

namespace fb::bot::integration {

attack_test::attack_test(game_bot_controller& controller) :
    bot_integration_test(controller)
{
    fb::logger::debug("Attack test constructed");
}

async::task<bool> attack_test::attack_scenario()
{
    auto bots = this->get_test_bots();

    if (bots.empty())
        throw std::runtime_error("No bots available for attack test");

    auto bot = bots.front();

    fb::logger::info("Attack test: Bot {} will perform attack sequences", bot->fd());

    // Perform attack sequences - simple attack to air 5 times
    constexpr int ATTACK_COUNT = 5;
    for (auto i = 0; i < ATTACK_COUNT; i++)
    {
        bot->send(fb::protocol::game::request::attack{});

        fb::logger::debug("Attack sequence {}: bot {} performed attack", i + 1, bot->fd());

        auto thread = bot->thread();
        co_await thread->sleep(DEFAULT_INTERVAL);
    }

    co_return true;
}

generator<bot_integration_test::scenario_t> attack_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->attack_scenario();
    };
}

std::string attack_test::name() const
{
    return "Attack Test";
}

} // namespace fb::bot::integration