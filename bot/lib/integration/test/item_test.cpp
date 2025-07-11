#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

item_test::item_test(game_bot_controller& controller) :
    bot_integration_test(controller)
{
    // Initialize test function queue
    this->initialize_test_functions();

    fb::logger::debug("Item test constructed");
}

async::task<bool> item_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto bots = this->get_test_bots();
    fb::logger::info("Starting item test with {} bots", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for item test");
        this->set_state(test_state::failed);
        co_return false;
    }

    // Step 1: Arrange bots in a line formation
    fb::logger::info("Arranging {} bots in line formation", bots.size());

    // Move bots in reverse order to avoid blocking (2→1→0)
    for (int i = static_cast<int>(bots.size()) - 1; i >= 1; --i)
    {
        auto& bot              = bots[i];
        auto  current_position = bot->position();
        auto  target_position  = current_position;
        auto  thread           = bot->thread();

        // Move bot i steps to the right
        co_await thread->switching();
        co_await bot->move(DIRECTION::RIGHT, i, DEFAULT_INTERVAL);

        // Set direction to BOTTOM
        bot->send(fb::protocol::game::request::direction{DIRECTION::BOTTOM});

        fb::logger::debug("Bot {} positioned at ({}, {}) facing BOTTOM",
                          bot->fd(),
                          target_position.x,
                          target_position.y);
    }

    fb::logger::info("Bot line formation completed");

    // Step 2: Execute all registered test functions
    auto test_result = co_await this->execute_test_functions(bots);
    if (test_result == false)
    {
        fb::logger::fatal("One or more test functions failed");
        co_return false;
    }

    this->set_state(test_state::completed);

    fb::logger::info("Item test completed successfully");

    // Notify controller that this test is completed
    this->notify_completed();

    co_return true;
}

void item_test::reset()
{
    // Call base class reset
    bot_integration_test::reset();

    // Re-initialize test functions
    this->initialize_test_functions();

    fb::logger::info("Item test reset");
}

void item_test::initialize_test_functions()
{
    // Clear existing test functions
    this->_test_functions.clear();

    // Register all test functions in execution order
    this->_test_functions.emplace_back("Equipment Success Test", [this](auto& bots) {
        return this->test_equipment_success();
    });

    this->_test_functions.emplace_back("Equipment Failure Test", [this](auto& bots) {
        return this->test_equipment_failure();
    });

    this->_test_functions.emplace_back("Equipment Overflow Test", [this](auto& bots) {
        return this->test_equipment_overflow();
    });

    fb::logger::info("Initialized {} item test functions", this->_test_functions.size());
}

async::task<bool> item_test::execute_test_functions(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots)
{
    fb::logger::info("Executing {} item test functions", this->_test_functions.size());

    for (size_t i = 0; i < this->_test_functions.size(); ++i)
    {
        const auto& [test_name, test_func] = this->_test_functions[i];
        fb::logger::info("Executing test function {} of {}: {}", i + 1, this->_test_functions.size(), test_name);

        auto result = co_await test_func(bots);
        if (result == false)
        {
            fb::logger::fatal("Test function '{}' failed", test_name);
            co_return false;
        }

        fb::logger::info("Test function '{}' completed successfully", test_name);

        // Reset bot state after each test function completes
        for (auto& bot : bots)
        {
            fb::logger::debug("Resetting bot state after test function '{}'", test_name);
            co_await this->reset_bot_state(bot);
        }
    }

    fb::logger::info("All item test functions completed successfully");
    co_return true;
}

async::task<void> item_test::reset_bot_state(std::shared_ptr<fb::bot::game_bot>& bot)
{
    auto thread = bot->thread();
    co_await thread->switching();

    fb::logger::debug("Resetting bot {} state to clean initial conditions", bot->oid());

    // Clear all items from inventory
    bot->chat("/아이템초기화");
    co_await bot->thread()->sleep(500ms);

    // Reset money to 0
    co_await bot->change_money(0, DEFAULT_TIMEOUT);

    // Reset to default level (level 1)
    co_await bot->change_level(1, DEFAULT_TIMEOUT);

    // Reset all stats to 1
    co_await bot->change_stats(1, 1, 1, DEFAULT_TIMEOUT);

    // Reset sex to male
    co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

    // Reset base HP and MP to reasonable defaults
    co_await bot->change_base_hp(1000, DEFAULT_TIMEOUT);
    co_await bot->change_base_mp(1000, DEFAULT_TIMEOUT);

    fb::logger::debug("Bot {} state reset completed", bot->oid());
}

std::string item_test::name() const
{
    return "Item Test";
}

} // namespace fb::bot::integration