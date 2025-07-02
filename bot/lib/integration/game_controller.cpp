#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/movement_test.h>
#include <fb/bot/integration/attack_test.h>
#include <fb/bot/integration/skill_test.h>
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

    // Initialize test queue with sequence of tests
    this->_test_queue.push(std::make_unique<movement_test>());
    this->_test_queue.push(std::make_unique<attack_test>());
    this->_test_queue.push(std::make_unique<skill_test>());

    // Start the first test
    this->start_next_test();

    fb::logger::info("Integration test controller initialized with test queue (movement -> attack -> skill)");
}

void game_bot_controller::set_test(std::unique_ptr<bot_integration_test> test)
{
    this->_current_test.write([&](auto& current_test) {
        if (current_test)
        {
            if (current_test->is_running())
            {
                fb::logger::warn("Stopping current test '{}' to set new test", current_test->name());
            }

            // Cleanup previous test bots
            current_test->cleanup();
        }

        current_test = std::move(test);

        if (current_test)
        {
            current_test->reset();
            fb::logger::info("Test case set to: {}", current_test->name());

            async::awaitable_then(current_test->initialize(*this), [test_name = current_test->name()](auto result) {
                try
                {
                    result();
                    fb::logger::info("Test '{}' initialization completed", test_name);
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("Failed to initialize test '{}': {}", test_name, e.what());
                }
            });
        }
    });
}

async::task<bool> game_bot_controller::start_test()
{
    co_return co_await this->_current_test.async_write([](auto& current_test) -> async::task<bool> {
        if (!current_test)
        {
            fb::logger::warn("No test case is currently set");
            co_return false;
        }

        if (current_test->is_running())
        {
            fb::logger::warn("Test '{}' is already running", current_test->name());
            co_return false;
        }

        if (current_test->is_complete())
        {
            fb::logger::info("Resetting completed test '{}'", current_test->name());
            current_test->reset();
        }

        fb::logger::info("Starting test '{}'", current_test->name());
        auto result = co_await current_test->execute();
        co_return result;
    });
}

void game_bot_controller::reset_current_test()
{
    this->_current_test.write([](auto& current_test) {
        if (current_test)
        {
            current_test->reset();
            fb::logger::info("Test '{}' has been reset", current_test->name());
        }
    });
}

void game_bot_controller::start_next_test()
{
    if (this->_test_queue.empty())
    {
        fb::logger::info("All integration tests completed successfully!");
        return;
    }

    // Get next test from queue
    auto next_test = std::move(this->_test_queue.front());
    this->_test_queue.pop();

    // Set as current test (this will also initialize it)
    this->set_test(std::move(next_test));
}

async::task<void> game_bot_controller::handle_timer()
{
    bool        should_start = false;
    std::string test_name;

    // Check test state with minimal lock time to avoid deadlock
    this->_current_test.read([&](const auto& current_test) {
        if (!current_test)
            return;

        test_name    = current_test->name();
        should_start = !current_test->is_running() && !current_test->is_complete() && current_test->is_ready();
    });

    // Handle test start (outside of lock to avoid deadlock)
    if (should_start)
    {
        fb::logger::info("All bots are ready for '{}', starting test", test_name);

        async::awaitable_then(this->start_test(), [this, test_name](auto result) {
            try
            {
                auto success = result();
                if (success)
                {
                    fb::logger::info("Test '{}' completed successfully, starting next test", test_name);
                    this->start_next_test();
                }
                else
                {
                    fb::logger::fatal("Test '{}' failed, stopping test sequence", test_name);
                }
            }
            catch (std::exception& e)
            {
                fb::logger::warn("Failed to execute test '{}': {}", test_name, e.what());
            }
        });
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
    // Notify current test about bot connection
    this->_current_test.write([this, &bot](auto& current_test) {
        if (current_test)
        {
            // Use shared_from_this to get shared_ptr to game_bot
            auto bot_shared = this->_bots.read([&bot](const auto& bots) {
                auto it = bots.find(bot.id);
                return (it != bots.end()) ? it->second : nullptr;
            });

            if (bot_shared)
            {
                // Let each test decide whether to store this bot or not
                current_test->on_bot_connected(bot_shared);
                fb::logger::debug("Game bot {} connection notified to test '{}'", bot.id, current_test->name());
            }
        }
    });

    fb::logger::info("Bot {} connected for integration testing", bot.fd());

    // Integration test: Initialize test scenarios upon connection
    bot.send(fb::protocol::game::request::login(bot.transfer_buffer()), false, true);

    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    fb::logger::info("Bot {} disconnected from integration testing", bot.fd());

    // Integration test: Collect test results and perform cleanup
    // TODO: Generate test report for this bot session
    co_return;
}