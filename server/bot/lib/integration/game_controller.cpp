#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/movement_test.h>
#include <fb/bot/integration/attack_test.h>
#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/bulletin_test.h>
#include <fb/bot/integration/trade_test.h>
#include <fb/bot/integration/communication_test.h>
#include <fb/bot/integration/drop_loot_test.h>
#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/item_test.give.h>
#include <fb/bot/integration/emotion_test.h>
#include <fb/bot/integration/front_info_test.h>
#include <fb/bot/integration/chat_interaction_test.h>
#include <fb/bot/integration/user_list_test.h>
#include <fb/bot/integration/swap_test.h>
#include <fb/bot/integration/throw_test.h>
#include <fb/bot/integration/worldmap_test.h>
#include <fb/bot/integration/door_test.h>
#include <fb/bot/integration/group_test.h>
#include <fb/bot/integration/clan_test.h>
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
    this->bind(&game_bot_controller::on_time);
    this->bind(&game_bot_controller::on_state);
    this->bind(&game_bot_controller::on_message);
    this->bind(&game_bot_controller::on_sequence);
    this->bind(&game_bot_controller::on_position);
    this->bind(&game_bot_controller::on_move);
    this->bind(&game_bot_controller::on_map);
    this->bind(&game_bot_controller::on_transfer);
}

void game_bot_controller::initialize()
{
    table::item.hook.build = [](const Json::Value& json) -> fb::model::item* {
        auto type = fb::model::build<ITEM_TYPE>(json["type"]);
        switch (type)
        {
        case ITEM_TYPE::STUFF:
            return fb::model::build<fb::model::item*>(json);
        case ITEM_TYPE::CASH:
            return fb::model::build<fb::model::cash*>(json);
        case ITEM_TYPE::CONSUME:
            return fb::model::build<fb::model::consume*>(json);
        case ITEM_TYPE::WEAPON:
            return fb::model::build<fb::model::weapon*>(json);
        case ITEM_TYPE::ARMOR:
            return fb::model::build<fb::model::armor*>(json);
        case ITEM_TYPE::HELMET:
            return fb::model::build<fb::model::helmet*>(json);
        case ITEM_TYPE::RING:
            return fb::model::build<fb::model::ring*>(json);
        case ITEM_TYPE::SHIELD:
            return fb::model::build<fb::model::shield*>(json);
        case ITEM_TYPE::AUXILIARY:
            return fb::model::build<fb::model::auxiliary*>(json);
        case ITEM_TYPE::PACKAGE:
            return fb::model::build<fb::model::pack*>(json);
        default:
            return nullptr;
        }
    };

    fb::model::loader().run();

    // Set up integration test timer with different interval (slower for detailed testing)
    this->bind_timer(&game_bot_controller::on_timer, 1000ms);

    auto local = fb::config<std::string_view>("ip") == "127.0.0.1";

    // Create tests and add them to the queue
    this->enqueue_test(std::make_unique<movement_test>(*this));
    this->enqueue_test(std::make_unique<attack_test>(*this));
    this->enqueue_test(std::make_unique<skill_test>(*this));
    this->enqueue_test(std::make_unique<bulletin_test>(*this));
    this->enqueue_test(std::make_unique<trade_test>(*this));
    this->enqueue_test(std::make_unique<communication_test>(*this));
    this->enqueue_test(std::make_unique<drop_loot_test>(*this));
    this->enqueue_test(std::make_unique<item_test>(*this));
    this->enqueue_test(std::make_unique<item_test_give>(*this));
    this->enqueue_test(std::make_unique<emotion_test>(*this));
    this->enqueue_test(std::make_unique<front_info_test>(*this));
    this->enqueue_test(std::make_unique<chat_interaction_test>(*this));
    this->enqueue_test(std::make_unique<user_list_test>(*this));
    this->enqueue_test(std::make_unique<swap_test>(*this));
    this->enqueue_test(std::make_unique<throw_test>(*this));
    this->enqueue_test(std::make_unique<group_test>(*this));
    this->enqueue_test(std::make_unique<clan_test>(*this));
    if (!local)
    {
        this->enqueue_test(std::make_unique<worldmap_test>(*this));
        this->enqueue_test(std::make_unique<door_test>(*this));
    }

    // Log the test queue in a more manageable format
    fb::logger::info("Integration test controller initialized with {} tests in queue", this->_test_queue.size());
    fb::logger::info("Test execution order:");

    auto temp_queue  = this->_test_queue;
    int  test_number = 1;
    while (!temp_queue.empty())
    {
        fb::logger::info("  {}. {}", test_number++, temp_queue.front()->name());
        temp_queue.pop();
    }

    this->active_test();
}

async::task<void> game_bot_controller::active_test()
{
    if (this->_current_test == nullptr)
    {
        fb::logger::warn("No tests to activate");
        co_return;
    }

    fb::logger::debug("Activating first test: '{}'", this->_current_test->name());
    std::ignore = this->_current_test->on_activated(*this);
    fb::logger::debug("Test '{}' activated and ready to receive bot connections", this->_current_test->name());
}

void game_bot_controller::notify_test_ready()
{
    if (this->_current_test)
    {
        fb::logger::debug("Test '{}' is ready, starting execution", this->_current_test->name());
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

    // Store test result
    test_result result;
    result.name    = this->_current_test->name();
    result.success = success;
    result.message = success ? "PASSED" : "FAILED";
    this->_test_results.push_back(result);

    if (success)
        fb::logger::info(fb::console::color::light_green,
                         "Test '{}' completed successfully",
                         this->_current_test->name());
    else
        fb::logger::fatal(fb::console::color::light_red, "Test '{}' failed", this->_current_test->name());

    this->_current_test = nullptr;
    this->start_next_test();
}

async::task<void> game_bot_controller::on_timer()
{
    // Timer is only used for starting the first test from the queue
    // Subsequent tests are started automatically by the test chain
    static bool first_test_started = false;

    if (!first_test_started)
    {
        first_test_started = true;
        // No longer need to start tests - they start automatically via hooks
        fb::logger::debug("Integration test controller initialized - tests will start automatically");
    }

    co_return;
}

async::task<void> game_bot_controller::on_time(game_bot& bot, const game_resp::time& response)
{
    // Integration test: Validate time synchronization
    // TODO: Add time validation logic
    co_return;
}

async::task<void> game_bot_controller::on_state(game_bot& bot, const game_resp::update_internal& response)
{
    // Integration test: Validate state consistency
    // TODO: Add state validation logic
    co_return;
}

async::task<void> game_bot_controller::on_message(game_bot& bot, const game_resp::message& response)
{
    // Integration test: Validate message handling and trigger test responses
    if (response.type == MESSAGE_TYPE::NOTIFY)
    {
        // TODO: Parse message and execute appropriate test case
        // Example: Test command processing, NPC interactions, etc.
    }
    co_return;
}

async::task<void> game_bot_controller::on_sequence(game_bot& bot, const game_resp::id& response)
{
    // Integration test: Validate object ID consistency
    bot.set_oid(response.oid);

    co_return;
}

async::task<void> game_bot_controller::on_position(game_bot& bot, const game_resp::position& response)
{
    // Integration test: Validate position updates
    bot.set_position(response.abs);
    // TODO: Add position validation logic
    co_return;
}

async::task<void> game_bot_controller::on_move(game_bot& bot, const game_resp::move& response)
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

async::task<void> game_bot_controller::on_map(game_bot& bot, const game_resp::map_config& response)
{
    // TODO: Execute map-specific test scenarios
    // Example: Test NPC interactions, item spawning, area transitions, etc.
    co_return;
}

async::task<void> game_bot_controller::on_transfer(game_bot& bot, const fb::protocol::response::transfer& response)
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
    fb::logger::debug("Bot {} connected for integration testing", bot.fd());

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
    fb::logger::debug("Bot {} disconnected from integration testing", bot.name());

    // Integration test: Collect test results and perform cleanup
    // TODO: Generate test report for this bot session
    co_return;
}

async::task<void> game_bot_controller::on_integration_hook_execution(uint8_t                     opcode,
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
        auto  cmd_it     = test_hooks.find(opcode);
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
}

void game_bot_controller::start_next_test()
{
    if (this->_test_queue.empty())
    {
        fb::logger::debug("No more tests in queue");
        return;
    }

    // Remove the completed test from the queue
    this->_test_queue.pop();

    if (this->_test_queue.empty())
    {
        this->print_final_test_results();
        this->container.exit();
        return;
    }

    // Set the next test as current
    this->_current_test = this->_test_queue.front();
    fb::logger::debug("Starting next test: '{}'", this->_current_test->name());

    // Activate the next test
    std::ignore = this->active_test();
}

bool game_bot_controller::has_more_tests() const
{
    return !this->_test_queue.empty();
}

void game_bot_controller::print_final_test_results()
{
    fb::logger::info(fb::console::color::cyan, "=== INTEGRATION TEST RESULTS ===");

    int total_tests  = this->_test_results.size();
    int passed_tests = 0;
    int failed_tests = 0;

    // Print individual test results
    for (const auto& result : this->_test_results)
    {
        if (result.success)
        {
            fb::logger::info(fb::console::color::light_green, "[PASS] {}: {}", result.name, result.message);
            passed_tests++;
        }
        else
        {
            fb::logger::fatal(fb::console::color::light_red, "[FAIL] {}: {}", result.name, result.message);
            failed_tests++;
        }
    }

    // Print summary
    fb::logger::info(fb::console::color::cyan, "=== SUMMARY ===");
    fb::logger::info(fb::console::color::light_blue, "Total tests: {}", total_tests);
    fb::logger::info(fb::console::color::light_green, "Passed: {}", passed_tests);
    fb::logger::info(fb::console::color::light_red, "Failed: {}", failed_tests);

    // Print overall result
    if (failed_tests == 0)
    {
        fb::logger::info(fb::console::color::light_green,
                         "ALL TESTS PASSED! Integration test suite completed successfully.");
    }
    else
    {
        fb::logger::fatal(fb::console::color::light_red,
                          "{} TEST(S) FAILED! Integration test suite has failures.",
                          failed_tests);
    }

    fb::logger::info(fb::console::color::cyan, "=== END OF INTEGRATION TEST RESULTS ===");
}