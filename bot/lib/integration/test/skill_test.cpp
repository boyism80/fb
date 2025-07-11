#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

skill_test::skill_test(game_bot_controller& controller) :
    bot_integration_test(controller, 5) // Spawn 5 bots
{
    // Initialize test function queue
    this->initialize_test_functions();

    fb::logger::debug("Skill test constructed");
}

async::task<bool> skill_test::execute()
{
    if (this->get_state() == test_state::running || this->get_state() == test_state::completed)
        co_return false;

    this->set_state(test_state::running);

    auto bots = this->get_test_bots();
    fb::logger::info("Starting skill test with {} bot", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for skill test");
        this->set_state(test_state::failed);
        co_return false;
    }

    // Step 1: Arrange bots in a line formation
    fb::logger::info("Arranging {} bots in line formation", bots.size());

    // Move bots in reverse order to avoid blocking (4→3→2→1)
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

    fb::logger::info("Skill test completed successfully");

    // Notify controller that this test is completed
    this->notify_completed();

    co_return true;
}

void skill_test::reset()
{
    // Call base class reset
    bot_integration_test::reset();

    // Re-initialize test functions
    this->initialize_test_functions();

    fb::logger::info("Skill test reset");
}

void skill_test::initialize_test_functions()
{
    // Clear existing test functions
    this->_test_functions.clear();

    // Register all test functions in execution order
    this->_test_functions.emplace_back("Healing Spells", [this](auto& bots) {
        return this->test_healing_spells(bots);
    });

    this->_test_functions.emplace_back("Group Healing Spells", [this](auto& bots) {
        return this->test_group_healing_spells(bots);
    });

    this->_test_functions.emplace_back("Damage Spells", [this](auto& bots) {
        return this->test_damage_spells(bots);
    });

    this->_test_functions.emplace_back("Near Damage Spells", [this](auto& bots) {
        return this->test_near_damage_spells(bots);
    });

    this->_test_functions.emplace_back("Near Target Damage Spells", [this](auto& bots) {
        return this->test_near_target_damage_spells(bots);
    });

    this->_test_functions.emplace_back("Attack Cast Spells", [this](auto& bots) {
        return this->test_attack_cast_spells(bots);
    });

    this->_test_functions.emplace_back("Multi-Target Attack Cast Spells", [this](auto& bots) {
        return this->test_multi_target_attack_cast_spells(bots);
    });

    this->_test_functions.emplace_back("Area Damage Spells", [this](auto& bots) {
        return this->test_area_damage_spells(bots);
    });

    this->_test_functions.emplace_back("Buff/Debuff Spells", [this](auto& bots) {
        return this->test_buff_debuff_spells(bots);
    });

    this->_test_functions.emplace_back("Teleport Spells", [this](auto& bots) {
        return this->test_teleport_spells(bots);
    });

    this->_test_functions.emplace_back("Disguise Spells", [this](auto& bots) {
        return this->test_disguise_spells(bots);
    });

    this->_test_functions.emplace_back("Shout Spells", [this](auto& bots) {
        return this->test_shout_spells(bots);
    });

    this->_test_functions.emplace_back("Loot Spell", [this](auto& bots) {
        return this->test_loot_spell(bots);
    });

    fb::logger::info("Initialized {} test functions", this->_test_functions.size());
}

async::task<bool> skill_test::execute_test_functions(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots)
{
    fb::logger::info("Executing {} test functions", this->_test_functions.size());

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

        // Clean up all spells after each test function completes
        if (bots.empty() == false)
        {
            auto& caster = bots.front(); // Use first bot as caster for cleanup
            fb::logger::debug("Cleaning up all spells after test function '{}'", test_name);
            co_await caster->clear_all_spells(DEFAULT_TIMEOUT);

            fb::logger::debug("Clearing all items after test function '{}'", test_name);
            co_await caster->clear_all_items(DEFAULT_TIMEOUT);
            co_await caster->thread()->sleep(500ms);
        }
    }

    fb::logger::info("All test functions completed successfully");
    co_return true;
}

std::string skill_test::name() const
{
    return "Skill Test";
}

} // namespace fb::bot::integration