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

async::task<bool> attack_test::execute()
{
    constexpr auto interval = 100ms;

    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto bots = this->get_test_bots();
    fb::logger::info("Starting attack test with {} bots", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for attack test");
        this->set_state(test_state::failed);
        co_return false;
    }

    auto bot = bots.front();

    fb::logger::info("Attack test: Bot {} will perform attack sequences", bot->fd());

    // Perform attack sequences - simple attack to air 5 times
    constexpr int ATTACK_COUNT = 5;
    for (auto i = 0; i < ATTACK_COUNT; i++)
    {
        bot->send(fb::protocol::game::request::attack{});

        fb::logger::debug("Attack sequence {}: bot {} performed attack", i + 1, bot->fd());

        auto thread = bot->thread();
        co_await thread->switching();
        co_await thread->sleep(interval);
    }

    this->set_state(test_state::completed);

    fb::logger::info("Attack test completed successfully");

    // Notify controller that this test is completed
    this->notify_completed();

    co_return true; // 성공
}

void attack_test::reset()
{
    this->set_state(test_state::idle);
    fb::logger::info("Attack test reset");
}

std::string attack_test::name() const
{
    return "Attack Test";
}

} // namespace fb::bot::integration