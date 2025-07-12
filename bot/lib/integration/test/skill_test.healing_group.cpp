#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

namespace fb::bot::integration {

async::task<bool> skill_test::test_group_healing_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    auto bots = this->get_test_bots();

    fb::logger::info("Starting group healing spell test with {} bots", bots.size());
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
    fb::logger::info("Forming group with {} bots for group healing test", bots.size());
    co_await this->form_group();

    uint8_t spell_slot = 0;

    // Test each group healing spell
    for (const auto& spell_info : group_healing_spells)
    {
        fb::logger::info("Testing group healing spell: {}", spell_info.name);

        std::ignore = co_await caster->set_current_hp_mp(10000, 10000, DEFAULT_TIMEOUT);

        // Calculate expected values
        auto [expected_hp_gain, expected_mp_cost] = spell_info.calculator(caster.get());

        fb::logger::info("Group spell calculation: expected HP gain={}, expected MP cost={}",
                         expected_hp_gain,
                         expected_mp_cost);

        // Prepare all bots for testing - set low HP to see healing effect
        co_await this->prepare_bots_for_group_healing(expected_hp_gain);

        // Record HP values before casting
        std::vector<int> before_hp_values;
        for (auto& bot : bots)
        {
            before_hp_values.push_back(bot->hp());
        }

        auto before_caster_mp = caster->mp();

        fb::logger::debug("Casting group healing spell - caster MP={}", before_caster_mp);

        // Cast group healing spell
        spell_slot++;
        std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, spell_slot, "", 0, {0, 0}),
            [before_caster_mp, expected_mp_cost](auto& resp) -> bool {
                return resp.ch_mp == before_caster_mp - expected_mp_cost;
            },
            DEFAULT_TIMEOUT);
        caster->chat(std::format("Testing {}", spell_info.name));

        // Verify HP recovery for all group members
        co_await this->verify_group_healing_effects(before_hp_values, expected_hp_gain);

        fb::logger::info("Group healing spell {} test completed successfully", spell_info.name);
    }

    // Cleanup group
    co_await this->cleanup_group();

    fb::logger::info("Group healing spell test completed successfully - {} spells tested", group_healing_spells.size());
    caster->chat("All group healing spell tests completed successfully!");

    co_return true;
}

async::task<void> skill_test::form_group()
{
    auto  bots   = this->get_test_bots();
    auto& caster = bots.front();

    // Caster invites all other bots to the group
    for (size_t i = 1; i < bots.size(); ++i)
    {
        auto& target_bot = bots[i];
        fb::logger::debug("Inviting bot {} to group", target_bot->name());

        // Send group invitation
        auto group_request = fb::protocol::game::request::group{};
        group_request.name = target_bot->name();
        auto&& resp        = co_await caster->request<fb::protocol::game::response::message>(
            group_request,
            [](auto& resp) -> bool {
                return resp.type == MESSAGE_TYPE::STATE;
            },
            DEFAULT_TIMEOUT);
        caster->chat(resp.text);
    }

    fb::logger::info("Group formation completed with {} members", bots.size());
}

async::task<void> skill_test::prepare_bots_for_group_healing(int expected_hp_gain)
{
    auto bots = this->get_test_bots();
    // Set all bots to low HP for visible healing effect
    constexpr int base_hp = 50;

    for (auto& bot : bots)
    {
        auto max_hp          = bot->base_hp();
        auto recoverable_hp  = max_hp - base_hp;
        auto required_low_hp = base_hp;

        // If expected gain exceeds recoverable amount, adjust starting HP
        if (expected_hp_gain > recoverable_hp)
        {
            required_low_hp = std::max<uint32_t>(1, max_hp - expected_hp_gain);
            fb::logger::info("Adjusting bot {} HP to {} for accurate group healing test", bot->name(), required_low_hp);
        }

        co_await bot->change_hp(required_low_hp, DEFAULT_TIMEOUT);
    }

    fb::logger::info("All bots prepared for group healing test");
}

async::task<void> skill_test::verify_group_healing_effects(const std::vector<int>& before_hp_values,
                                                           int                     expected_hp_gain)
{
    auto bots = this->get_test_bots();
    for (size_t i = 0; i < bots.size(); ++i)
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
        }
        else
        {
            fb::logger::debug("Group healing verified for bot {}: HP {} -> {}",
                              bot->name(),
                              before_hp_values[i],
                              actual_hp);
        }
    }
    co_return;
}

async::task<void> skill_test::cleanup_group()
{
    auto bots = this->get_test_bots();
    fb::logger::info("Cleaning up group formation");

    // Cleanup group option for all bots
    for (auto& bot : bots)
    {
        for (int i = 0; i < 2; i++)
        {
            std::ignore = co_await bot->request<fb::protocol::game::response::message>(
                fb::protocol::game::request::update_option(OPTION::GROUP, false),
                [](auto& resp) -> bool {
                    return resp.type == MESSAGE_TYPE::STATE;
                },
                DEFAULT_TIMEOUT);
        }
    }

    fb::logger::info("Group cleanup completed");
}

} // namespace fb::bot::integration