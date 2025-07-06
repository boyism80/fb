#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

skill_test::skill_test(game_bot_controller& controller) :
    bot_integration_test(controller)
{
    // Register hook for object ID (sequence) responses
    this->_controller.hook_external(this, this, &skill_test::on_hook_sequence);
    this->_controller.hook_external(this, this, &skill_test::on_hook_position);

    // Initialize test function queue
    this->initialize_test_functions();

    fb::logger::debug("Skill test constructed");
}

async::task<void> skill_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 5;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Skill test initializing and spawning {} bot", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    // Initialize test function queue
    this->initialize_test_functions();

    fb::logger::info("Skill test initialization completed - {} bot spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> skill_test::execute()
{
    constexpr auto interval = 100ms; // Increased from 100ms to reduce server load
#if defined DEBUG || defined _DEBUG
    constexpr auto timeout = 1h;
#else
    constexpr auto timeout = 5s;
#endif

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
        co_await bot->move(DIRECTION::RIGHT, i, 50ms);

        // Set direction to BOTTOM
        bot->send(fb::protocol::game::request::direction{DIRECTION::BOTTOM});

        fb::logger::debug("Bot {} positioned at ({}, {}) facing BOTTOM",
                          bot->fd(),
                          target_position.x,
                          target_position.y);
    }

    fb::logger::info("Bot line formation completed");

    // Step 2: Execute all registered test functions
    auto test_result = co_await this->execute_test_functions(bots, timeout);
    if (test_result == false)
    {
        fb::logger::fatal("One or more test functions failed");
        this->cleanup();
        co_return false;
    }

    this->set_state(test_state::completed);

    fb::logger::info("Skill test completed successfully");

    // Cleanup bots after test completion
    this->cleanup();

    // Notify controller that this test is completed
    this->_controller.notify_test_completed(this);

    co_return true; // 성공
}

void skill_test::reset()
{
    // Call base class reset
    bot_integration_test::reset();

    this->_waiting_for_spell_update = false;
    this->_spell_learned_index      = 0;

    // Re-initialize test functions
    this->initialize_test_functions();

    fb::logger::info("Skill test reset");
}

bool skill_test::is_ready() const
{
    if (this->get_test_bots().empty())
        return false;

    for (auto& bot : this->get_test_bots())
    {
        if (bot->oid() == 0)
            return false;

        if (bot->position().x == 0 && bot->position().y == 0)
            return false;
    }

    return true;
}

void skill_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    // Call base class implementation
    bot_integration_test::on_bot_connected(bot);
    fb::logger::debug("Skill test: Bot {} added to collection", bot->fd());
}

void skill_test::on_spell_update_received(std::shared_ptr<fb::bot::game_bot> bot, uint8_t index)
{
    fb::logger::info("Skill test: Bot {} received spell_update with index {}", bot->fd(), index);

    this->_spell_learned_index      = index;
    this->_waiting_for_spell_update = false;
}

async::task<void> skill_test::on_hook_sequence(fb::bot::game_bot& bot, const fb::protocol::game::response::id& response)
{
    fb::logger::debug("Skill test: Bot {} received object ID {}", bot.fd(), response.oid);

    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Skill test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

async::task<void> skill_test::on_hook_position(fb::bot::game_bot&                            bot,
                                               const fb::protocol::game::response::position& response)
{
    if (this->is_ready() == false)
        co_return;

    if (this->get_state() == test_state::running)
        co_return;

    fb::logger::info("Skill test: All bots ready, notifying controller");
    this->set_state(test_state::ready);
    this->notify_ready();
    co_return;
}

void skill_test::initialize_test_functions()
{
    // Clear existing test functions
    this->_test_functions.clear();

    // Register all test functions in execution order
    this->_test_functions.emplace_back("Healing Spells", [this](auto& bots, auto timeout) {
        return this->test_healing_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Group Healing Spells", [this](auto& bots, auto timeout) {
        return this->test_group_healing_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Damage Spells", [this](auto& bots, auto timeout) {
        return this->test_damage_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Near Damage Spells", [this](auto& bots, auto timeout) {
        return this->test_near_damage_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Near Target Damage Spells", [this](auto& bots, auto timeout) {
        return this->test_near_target_damage_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Attack Cast Spells", [this](auto& bots, auto timeout) {
        return this->test_attack_cast_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Multi-Target Attack Cast Spells", [this](auto& bots, auto timeout) {
        return this->test_multi_target_attack_cast_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Area Damage Spells", [this](auto& bots, auto timeout) {
        return this->test_area_damage_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Buff/Debuff Spells", [this](auto& bots, auto timeout) {
        return this->test_buff_debuff_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Teleport Spells", [this](auto& bots, auto timeout) {
        return this->test_teleport_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Disguise Spells", [this](auto& bots, auto timeout) {
        return this->test_disguise_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Shout Spells", [this](auto& bots, auto timeout) {
        return this->test_shout_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Loot Spell", [this](auto& bots, auto timeout) {
        return this->test_loot_spell(bots, timeout);
    });

    fb::logger::info("Initialized {} test functions", this->_test_functions.size());
}

async::task<bool> skill_test::execute_test_functions(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                     std::chrono::milliseconds                        timeout)
{
    fb::logger::info("Executing {} test functions", this->_test_functions.size());

    for (size_t i = 0; i < this->_test_functions.size(); ++i)
    {
        const auto& [test_name, test_func] = this->_test_functions[i];
        fb::logger::info("Executing test function {} of {}: {}", i + 1, this->_test_functions.size(), test_name);

        auto result = co_await test_func(bots, timeout);
        if (result == false)
        {
            fb::logger::fatal("Test function '{}' failed", test_name);
            co_return false;
        }

        fb::logger::info("Test function '{}' completed successfully", test_name);

        // Clean up all spells after each test function completes
        if (bots.empty() == false)
        {
            auto& caster = bots[0]; // Use first bot as caster for cleanup
            fb::logger::debug("Cleaning up all spells after test function '{}'", test_name);
            co_await this->clear_all_spells(caster, timeout);

            fb::logger::debug("Clearing all items after test function '{}'", test_name);
            co_await this->clear_all_items(caster, timeout);
            co_await caster->thread()->sleep(500ms);
        }
    }

    fb::logger::info("All test functions completed successfully");
    co_return true;
}

} // namespace fb::bot::integration