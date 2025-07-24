#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

namespace fb::bot::integration {

async::task<bool> skill_test::test_group_healing_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    auto bots = this->get_test_bots();

    fb::logger::debug("Starting group healing spell test with {} bots", bots.size());
    caster->chat("Starting group healing spell test");

    // Group healing spells with their test parameters
    struct group_healing_spell
    {
        std::string name;
        std::function<std::tuple<int, int>(fb::bot::game_bot*)>
            calculator; // returns {expected_hp_gain, expected_mp_cost}
    };

    auto group_healing_spells = std::vector<group_healing_spell>{
        {"백호의희원'첨",
         [](fb::bot::game_bot* bot) {
             int current_mp = static_cast<int>(bot->mp());
             return std::tuple<int, int>{current_mp * 2, current_mp / 2}; // HP gain = MP*2, MP cost = MP/2
         }                         },
        {"신령의기원'첨", [](fb::bot::game_bot* bot) {
             std::ignore = bot;                        // Not used for fixed values
             return std::tuple<int, int>{10000, 1000}; // Fixed HP gain and MP cost
         }}
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : group_healing_spells)
    {
        spell_names.push_back(spell.name);
    }

    std::ignore = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);

    // Form group with all available bots
    fb::logger::debug("Forming group with {} bots for group healing test", bots.size());
    co_await this->form_group();

    uint8_t spell_slot = 0;

    // Test each group healing spell
    auto success = true;
    for (const auto& spell_info : group_healing_spells)
    {
        fb::logger::debug("Testing group healing spell: {}", spell_info.name);

        // Set caster HP/MP to 10000 for testing
        std::ignore = co_await caster->set_current_hp_mp(10000, 10000, DEFAULT_TIMEOUT);

        // Calculate expected values
        auto [expected_hp_gain, expected_mp_cost] = spell_info.calculator(caster.get());

        fb::logger::debug("Group spell calculation: expected HP gain={}, expected MP cost={}",
                          expected_hp_gain,
                          expected_mp_cost);

        // Prepare other bots: set max HP to 10000, current HP to 50
        for (size_t i = 1; i < bots.size(); ++i)
        {
            auto& bot = bots[i];
            co_await bot->change_base_hp(100000, DEFAULT_TIMEOUT);
            co_await bot->change_hp(50, DEFAULT_TIMEOUT);
        }

        // Record HP values before casting
        std::vector<int> before_hp_values;
        for (auto& bot : bots)
        {
            before_hp_values.push_back(bot->hp());
        }

        auto before_caster_mp = caster->mp();

        fb::logger::debug("Casting group healing spell - caster MP={}", before_caster_mp);

        // Cast group healing spell
        auto member_count  = bots.size() - 1;
        auto receive_count = 0;
        std::ignore        = co_await caster->request<fb::protocol::game::response::update_hp>(
            fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, spell_slot, "", 0, {0, 0}),
            [member_count, &receive_count](auto& resp) -> bool {
                receive_count++;
                return receive_count == member_count;
            },
            DEFAULT_TIMEOUT);
        caster->chat(std::format("Testing {}", spell_info.name));

        if (caster->mp() != before_caster_mp - expected_mp_cost)
        {
            fb::logger::warn("Group healing spell {} test failed - caster MP mismatch: expected {}, actual {}",
                             spell_info.name,
                             before_caster_mp - expected_mp_cost,
                             caster->mp());
            success = false;
            continue;
        }

        // Verify HP recovery for all group members
        this->sleep(500ms);
        if (co_await this->verify_group_healing_effects(before_hp_values, expected_hp_gain) == false)
        {
            fb::logger::warn("Group healing spell {} test failed - HP recovery verification failed", spell_info.name);
            success = false;
            continue;
        }

        fb::logger::debug("Group healing spell {} test completed successfully", spell_info.name);
        spell_slot++;
    }

    // Cleanup group
    co_await this->cleanup_group();

    fb::logger::debug("Group healing spell test completed successfully - {} spells tested",
                      group_healing_spells.size());
    caster->chat("All group healing spell tests completed successfully!");

    co_return true;
}

async::task<bool> skill_test::verify_group_healing_effects(const std::vector<int>& before_hp_values,
                                                           int                     expected_hp_gain)
{
    auto bots = this->get_test_bots();
    for (size_t i = 1; i < bots.size(); ++i)
    {
        auto& bot                = bots[i];
        auto  max_hp             = bot->base_hp();
        auto  recoverable_amount = max_hp - before_hp_values[i];
        auto  actual_hp_gain     = std::min<uint32_t>(expected_hp_gain, recoverable_amount);
        auto  expected_hp        = before_hp_values[i] + actual_hp_gain;
        auto  actual_hp          = bot->hp();

        if (actual_hp != expected_hp)
        {
            fb::logger::warn("Group healing verification failed for bot {}: expected HP={}, actual HP={}",
                             bot->name(),
                             expected_hp,
                             actual_hp);
            co_return false;
        }
    }
    co_return true;
}

} // namespace fb::bot::integration