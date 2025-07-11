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

async::task<void> bot_integration_test::on_finished()
{
    for (auto& bot : this->get_test_bots())
    {
        if (bot)
            bot->close();
    }

    fb::logger::info("{} test finished - all bots disconnected", this->name());
    co_return;
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

async::task<void> bot_integration_test::on_active(game_bot_controller& controller)
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

    auto scenario_generator = this->on_generate_scenario();
    while (scenario_generator.next())
    {
        auto scenario = scenario_generator.value();
        this->_scenario_queue.push(scenario);
    }

    co_return;
}

async::task<void> bot_integration_test::on_initialize(game_bot_controller& controller)
{
    auto bots = this->get_test_bots();
    for (int i = 0; i < bots.size(); i++)
    {
        auto& bot = bots[i];
        if (bot->position().x == 6 && bot->position().y == 6)
            continue;

        co_await bot->map_move("낙랑의방", 6, 6, DEFAULT_TIMEOUT);
    }
}

async::task<bool> bot_integration_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto failed         = false;
    auto scenario_index = 0;
    fb::logger::info("{}: Starting test execution", this->name());
    co_await this->on_initialize(this->_controller);
    while (this->_scenario_queue.empty() == false)
    {
        auto scenario = this->_scenario_queue.front();
        this->_scenario_queue.pop();
        try
        {
            co_await this->on_scenario_started(scenario_index);
            if (co_await scenario() == false)
                failed = true;
        }
        catch (const std::exception& e)
        {
            fb::logger::fatal("{}: Scenario {} failed: {}", this->name(), scenario_index, e.what());
            failed = true;
        }

        co_await this->on_scenario_finished(scenario_index);
        scenario_index++;
    }

    co_await this->on_finished();
    if (failed)
    {
        fb::logger::fatal("{}: Test failed", this->name());
        this->set_state(test_state::failed);
        co_return false;
    }

    fb::logger::info("{}: Test completed successfully", this->name());
    this->set_state(test_state::completed);
    co_return true;
}

async::task<void> bot_integration_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> bot_integration_test::on_scenario_finished(uint32_t scenario_index)
{
    co_return;
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