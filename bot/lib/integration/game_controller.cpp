#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/movement_test.h>
#include <fb/bot/integration/attack_test.h>
#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/bulletin_test.h>
#include <fb/bot/integration/trade_test.h>
#include <fb/bot/integration/communication_test.h>
#include <fb/bot/integration/drop_loot_test.h>
#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/swap_test.h>
#include <fb/bot/integration/throw_test.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/container.h>
#include <fb/bot/gateway_controller.h>
#include <fb/logger.h>

using namespace fb::bot::integration;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container),
    _current_test(nullptr)
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
    this->bind_timer(&game_bot_controller::handle_timer, 1000ms);

    // Create tests and add them to the queue
    this->enqueue_test(std::make_unique<movement_test>(*this));
    this->enqueue_test(std::make_unique<attack_test>(*this));
    this->enqueue_test(std::make_unique<skill_test>(*this));
    this->enqueue_test(std::make_unique<bulletin_test>(*this));
    this->enqueue_test(std::make_unique<trade_test>(*this));
    this->enqueue_test(std::make_unique<communication_test>(*this));
    this->enqueue_test(std::make_unique<drop_loot_test>(*this));
    this->enqueue_test(std::make_unique<item_test>(*this));
    this->enqueue_test(std::make_unique<swap_test>(*this));
    this->enqueue_test(std::make_unique<throw_test>(*this));

    fb::logger::info("Integration test controller initialized with test queue (movement -> attack -> skill -> bulletin "
                     "-> trade -> drop_loot -> item -> swap -> throw)");

    this->active_test();
}

async::task<void> game_bot_controller::active_test()
{
    if (this->_current_test == nullptr)
    {
        fb::logger::warn("No tests to activate");
        co_return;
    }

    fb::logger::info("Activating first test: '{}'", this->_current_test->name());
    std::ignore = this->_current_test->on_active(*this);
    fb::logger::info("Test '{}' activated and ready to receive bot connections", this->_current_test->name());
}

void game_bot_controller::notify_test_ready()
{
    if (this->_current_test)
    {
        fb::logger::info("Test '{}' is ready, starting execution", this->_current_test->name());
        std::ignore = this->start_current_test();
    }
}

async::task<void> game_bot_controller::start_current_test()
{
    if (!this->_current_test)
    {
        fb::logger::warn("No current test to start");
        co_return;
    }

    if (this->_current_test->is_running())
    {
        fb::logger::warn("Test '{}' is already running", this->_current_test->name());
        co_return;
    }

    fb::logger::info("Starting test '{}'", this->_current_test->name());
    auto success = co_await this->_current_test->execute();
    if (success)
        fb::logger::info("Test '{}' completed successfully", this->_current_test->name());
    else
        fb::logger::fatal("Test '{}' failed", this->_current_test->name());

    this->_current_test = nullptr;
    this->start_next_test();
}

async::task<void> game_bot_controller::handle_timer()
{
    // Timer is only used for starting the first test from the queue
    // Subsequent tests are started automatically by the test chain
    static bool first_test_started = false;

    if (!first_test_started)
    {
        first_test_started = true;
        // No longer need to start tests - they start automatically via hooks
        fb::logger::info("Integration test controller initialized - tests will start automatically");
    }

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
    // Integration test: Validate object ID consistency
    bot.set_oid(response.oid);

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
    if (bot.oid() != response.id)
    {
        // TODO: Log object ID mismatch for test analysis
        co_return;
    }

    bot.set_position(response.position);
    // TODO: Add movement validation logic
    co_return;
}

async::task<void> game_bot_controller::handle_map(game_bot&                                       bot,
                                                  const fb::protocol::game::response::map_config& response)
{
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
    fb::logger::info("Bot {} connected for integration testing", bot.fd());

    // Notify current test about bot connection
    if (this->_current_test)
    {
        // Use shared_from_this to get shared_ptr to game_bot
        auto bot_shared = this->_bots.read([&bot](const auto& bots) {
            auto it = bots.find(bot.id);
            return (it != bots.end()) ? it->second : nullptr;
        });

        if (bot_shared)
        {
            // Let the current test decide whether to store this bot or not
            this->_current_test->on_bot_connected(bot_shared);
            fb::logger::debug("Game bot {} connection notified to current test '{}'",
                              bot.id,
                              this->_current_test->name());
        }
    }

    // Integration test: Initialize test scenarios upon connection
    bot.send(fb::protocol::game::request::login(bot.transfer_buffer()), false, true);

    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    fb::logger::info("Bot {} disconnected from integration testing", bot.name());

    // Integration test: Collect test results and perform cleanup
    // TODO: Generate test report for this bot session
    co_return;
}

async::task<void> game_bot_controller::on_integration_hook_execution(uint8_t                     cmd,
                                                                     game_bot&                   bot,
                                                                     const fb::protocol::header& header)
{
    // Only execute hooks if there's a current test
    if (!this->_current_test)
        co_return;

    auto shared_lock = std::shared_lock<std::shared_mutex>(this->_hook_mutex);

    // Execute hooks only for the current test
    auto test_it = this->_test_hooks.find(this->_current_test);
    if (test_it != this->_test_hooks.end())
    {
        auto& test_hooks = test_it->second;
        auto  cmd_it     = test_hooks.find(cmd);
        if (cmd_it != test_hooks.end())
        {
            for (auto& hook : cmd_it->second)
            {
                co_await hook(bot, header);
            }
        }
    }
}

void game_bot_controller::enqueue_test(std::unique_ptr<bot_integration_test> test)
{
    // Store the test instance for lifetime management
    this->_test_instances.push_back(std::move(test));

    // Add the test to the queue
    this->_test_queue.push(this->_test_instances.back().get());

    // If this is the first test, set it as current
    if (this->_current_test == nullptr)
    {
        this->_current_test = this->_test_queue.front();
    }

    fb::logger::debug("Test '{}' added to queue", this->_test_instances.back()->name());
}

void game_bot_controller::start_next_test()
{
    if (this->_test_queue.empty())
    {
        fb::logger::info("No more tests in queue");
        return;
    }

    // Remove the completed test from the queue
    this->_test_queue.pop();

    if (this->_test_queue.empty())
    {
        fb::logger::info("All tests completed");
        this->container.exit();
        return;
    }

    // Set the next test as current
    this->_current_test = this->_test_queue.front();
    fb::logger::info("Starting next test: '{}'", this->_current_test->name());

    // Activate the next test
    std::ignore = this->active_test();
}

bool game_bot_controller::has_more_tests() const
{
    return !this->_test_queue.empty();
}