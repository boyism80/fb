#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/config.h>
#include <format>

using namespace std::chrono_literals;

namespace fb::bot::integration {

bot_integration_test::bot_integration_test(game_bot_controller& controller, uint32_t bot_count) :
    _controller(controller),
    bot_count(bot_count)
{
    // Register common hooks for sequence and position responses
    this->_controller.hook_external(this, this, &bot_integration_test::on_hook_sequence);
    this->_controller.hook_external(this, this, &bot_integration_test::on_hook_position);
}

bot_integration_test::test_state bot_integration_test::get_state() const
{
    return this->_state;
}

void bot_integration_test::set_state(test_state state)
{
    this->_state = state;
}

bool bot_integration_test::is_complete() const
{
    return this->_state == test_state::completed;
}

bool bot_integration_test::is_running() const
{
    return this->_state == test_state::running;
}

void bot_integration_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    this->_test_bots.push_back(bot);
    fb::logger::debug("{}: Bot {} added to collection", this->name(), bot->fd());
}

void bot_integration_test::on_bot_disconnected(std::shared_ptr<fb::bot::game_bot> bot)
{
    fb::logger::debug("{}: Bot {} disconnected", this->name(), bot->fd());
}

void bot_integration_test::notify_ready()
{
    this->_controller.notify_test_ready();
}

void bot_integration_test::notify_completed()
{
    this->_controller.notify_test_completed(this);
}

void bot_integration_test::cleanup()
{
    for (auto bot : this->get_test_bots())
    {
        if (bot)
            bot->close();
    }

    fb::logger::info("{} test cleanup completed - all bots disconnected", this->name());
}

bool bot_integration_test::is_ready() const
{
    auto bots = this->get_test_bots();
    if (bots.size() < this->bot_count)
        return false;

    for (auto& bot : bots)
    {
        if (bot->oid() == 0)
            return false;

        if (bot->position().x == 0 && bot->position().y == 0)
            return false;
    }

    return true;
}

std::vector<std::shared_ptr<fb::bot::game_bot>> bot_integration_test::get_test_bots() const
{
    return this->_test_bots;
}

async::task<void> bot_integration_test::initialize(game_bot_controller& controller)
{
    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("{} initializing and spawning {} bots", this->name(), this->bot_count);

    for (auto i = 0u; i < this->bot_count; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("{} initialization completed - {} bots spawned", this->name(), this->bot_count);
    co_return;
}

void bot_integration_test::reset()
{
    this->_state = test_state::idle;
}

async::task<void> bot_integration_test::on_hook_sequence(fb::bot::game_bot&                      bot,
                                                         const fb::protocol::game::response::id& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("{}: All bots ready, notifying controller", this->name());
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> bot_integration_test::on_hook_position(fb::bot::game_bot&                            bot,
                                                         const fb::protocol::game::response::position& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("{}: All bots ready, notifying controller", this->name());
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

} // namespace fb::bot::integration