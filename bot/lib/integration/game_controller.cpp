#include <fb/bot/integration/game_controller.h>
#include <fb/bot/game_bot.h>

using namespace fb::bot::integration;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container)
{
    // Bind integration test specific handlers
    this->bind(&game_bot_controller::handle_time);
    this->bind(&game_bot_controller::handle_state);
    this->bind(&game_bot_controller::handle_message);
    this->bind(&game_bot_controller::handle_sequence);
    this->bind(&game_bot_controller::handle_position);
    this->bind(&game_bot_controller::handle_move);
    this->bind(&game_bot_controller::handle_map);
    this->bind(&game_bot_controller::handle_transfer);
}

void game_bot_controller::initialize()
{
    // Set up integration test timer with different interval (slower for detailed testing)
    this->bind_thread_timer(&game_bot_controller::handle_timer, 1000ms);
}

async::task<void> game_bot_controller::handle_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->container.threads.at(id);
    auto params = thread->data<bot_thread_params>();
    if (params == nullptr)
        co_return;

    // Integration test logic: Execute test scenarios instead of random patterns
    this->_bots.read<void>([&](const auto& bots) {
        for (auto& [_, bot] : params->bots)
        {
            if (bots.contains(bot->id) == false)
                continue;

            auto typed_bot = static_cast<game_bot*>(bot.get());
            // TODO: Execute integration test scenarios here
            // typed_bot->execute_test_scenario(now);
        }
    });
    co_return;
}

async::task<void> game_bot_controller::handle_time(game_bot& bot, const fb::protocol::game::response::time& response)
{
    // Integration test: Validate time synchronization
    // TODO: Add time validation logic
    co_return;
}

async::task<void> game_bot_controller::handle_state(game_bot&                                            bot,
                                                    const fb::protocol::game::response::update_internal& response)
{
    // Integration test: Validate state consistency
    // TODO: Add state validation logic
    co_return;
}

async::task<void> game_bot_controller::handle_message(game_bot&                                    bot,
                                                      const fb::protocol::game::response::message& response)
{
    // Integration test: Validate message handling and trigger test responses
    if (response.type == MESSAGE_TYPE::NOTIFY)
    {
        // TODO: Parse message and execute appropriate test case
        // Example: Test command processing, NPC interactions, etc.
    }
    co_return;
}

async::task<void> game_bot_controller::handle_sequence(game_bot& bot, const fb::protocol::game::response::id& response)
{
    // Integration test: Validate sequence ID consistency
    bot.set_sequence(response.sequence);
    // TODO: Add sequence validation logic
    co_return;
}

async::task<void> game_bot_controller::handle_position(game_bot&                                     bot,
                                                       const fb::protocol::game::response::position& response)
{
    // Integration test: Validate position updates
    bot.set_position(response.abs);
    // TODO: Add position validation logic
    co_return;
}

async::task<void> game_bot_controller::handle_move(game_bot& bot, const fb::protocol::game::response::move& response)
{
    // Integration test: Validate movement mechanics
    if (bot.sequence() != response.id)
    {
        // TODO: Log sequence mismatch for test analysis
        co_return;
    }

    bot.set_position(response.position);
    // TODO: Add movement validation logic
    co_return;
}

async::task<void> game_bot_controller::handle_map(game_bot&                                       bot,
                                                  const fb::protocol::game::response::map_config& response)
{
    // Integration test: Validate map loading and configuration
    bot.set_initialized(true);

    // TODO: Execute map-specific test scenarios
    // Example: Test NPC interactions, item spawning, area transitions, etc.
    co_return;
}

async::task<void> game_bot_controller::handle_transfer(game_bot& bot, const fb::protocol::response::transfer& response)
{
    // Integration test: Validate server transfer mechanics
    bot.close();

    // TODO: Add transfer validation and test continuation logic
    auto created  = this->create(response.parameter);
    auto ip       = boost::asio::ip::address_v4(boost::endian::endian_reverse(response.ip));
    auto endpoint = boost::asio::ip::tcp::endpoint(ip, response.port);
    created->connect(endpoint);

    co_return;
}

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    // Integration test: Initialize test scenarios upon connection
    bot.send(fb::protocol::game::request::login(bot.transfer_buffer()), false, true);

    // TODO: Set up test scenario context for this bot
    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    // Integration test: Collect test results and perform cleanup
    // TODO: Generate test report for this bot session
    co_return;
}