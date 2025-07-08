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
    // Register hook for object ID (sequence) responses
    this->_controller.hook_external(this, this, &attack_test::on_hook_sequence);
    fb::logger::debug("Attack test constructed");
}

async::task<void> attack_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 1;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Attack test initializing and spawning {} bot", REQUIRED_BOTS);

    auto gateway_bot = controller.container.gateway->create();
    gateway_bot->connect(endpoint);

    fb::logger::info("Attack test initialization completed - {} bot spawned", REQUIRED_BOTS);
    co_return;
}

async::task<void> attack_test::on_hook_sequence(fb::bot::game_bot&                      bot,
                                                const fb::protocol::game::response::id& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Attack test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
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
    this->_controller.notify_test_completed(this);

    co_return true; // 성공
}

void attack_test::reset()
{
    this->set_state(test_state::idle);
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
    bot_integration_test::on_bot_connected(bot);
    fb::logger::debug("Attack test: Bot {} added to collection", bot->fd());
}

} // namespace fb::bot::integration