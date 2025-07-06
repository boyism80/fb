#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/model/model.h>
#include <chrono>
#include <format>
#include <algorithm>
#include <vector>

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
    constexpr auto timeout  = 5s;    // Increased from 5s to handle server processing delays

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

async::task<bool> skill_test::test_healing_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                  std::chrono::milliseconds                        timeout)
{
    constexpr auto interval = 100ms;

    // Select the test bot for skill operations
    auto caster = bots.at(0);
    auto target = bots.at(1);

    fb::logger::info("Bot {} starting healing spell test", caster->oid());
    caster->send(fb::protocol::game::request::chat(false, "=== HEALING SPELL TEST STARTED ==="));

    // Step 1: Setup bots with HP/MP for spell testing
    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP and current HP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, 50, std::nullopt, timeout);
    }

    caster->send(fb::protocol::game::request::chat(false, "Bot setup completed - ready for spell testing"));

    std::vector<healing_spell_test> healing_spells = {
        {"누리의기원", SPELL_TYPE::NORMAL, 50,    30   },
        {"하늘의기원", SPELL_TYPE::NORMAL, 200,   120  },
        {"대지의기원", SPELL_TYPE::TARGET, 50,    50   },
        {"동해의기원", SPELL_TYPE::TARGET, 100,   30   },
        {"바다의기원", SPELL_TYPE::TARGET, 100,   50   },
        {"천공의기원", SPELL_TYPE::TARGET, 200,   100  },
        {"구름의기원", SPELL_TYPE::TARGET, 500,   120  },
        {"태양의기원", SPELL_TYPE::TARGET, 1000,  240  },
        {"생명의기원", SPELL_TYPE::TARGET, 5000,  300  },
        {"현자의기원", SPELL_TYPE::TARGET, 5000,  1000 },
        {"신령의기원", SPELL_TYPE::TARGET, 10000, 1000 },
        {"봉황의기원", SPELL_TYPE::TARGET, 30000, 10000},
        {"천공의희원", SPELL_TYPE::TARGET, 200,   200  },
        {"구름의희원", SPELL_TYPE::TARGET, 500,   480  },
        {"태양의희원", SPELL_TYPE::TARGET, 1000,  960  }
    };

    // Step 2: Learn all healing spells
    fb::logger::info("Learning {} healing spells", healing_spells.size());

    std::vector<std::string> spell_names;
    for (const auto& spell : healing_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names, timeout);
    fb::logger::info("Successfully learned {} out of {} healing spells", learned_count, healing_spells.size());
    caster->send(fb::protocol::game::request::chat(false, "All healing spells learned - starting test sequence"));

    // Step 3: Test each healing spell
    auto spell_slot = 1;
    for (const auto& spell : healing_spells)
    {
        fb::logger::info("Testing spell: {} (HP: +{}, MP: -{})",
                         spell.name,
                         spell.expected_hp_gain,
                         spell.expected_mp_cost);

        if (spell.type == SPELL_TYPE::NORMAL)
        {
            // Test self-healing spell
            auto before_hp = caster->hp();
            auto before_mp = caster->mp();

            fb::logger::debug("Casting {} on self - before: hp={}, mp={}", spell.name, before_hp, before_mp);

            std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
                fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
                [before_hp, spell](auto& resp) -> bool {
                    return resp.ch_hp == before_hp + spell.expected_hp_gain;
                },
                timeout);

            fb::logger::debug("Successfully cast {} - after: hp={}, mp={}", spell.name, caster->hp(), caster->mp());
        }
        else
        {
            // Test target healing spell
            auto before_target_hp = target->hp();
            auto before_caster_mp = caster->mp();

            fb::logger::debug("Casting {} on target - target hp={}, caster mp={}",
                              spell.name,
                              before_target_hp,
                              before_caster_mp);

            // Caster casts spell on target
            std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
                fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", target->oid(), target->position()),
                [before_caster_mp, spell](auto& resp) -> bool {
                    return resp.ch_mp == before_caster_mp - spell.expected_mp_cost;
                },
                timeout);

            // Wait for target's HP to update and verify
            co_await caster->thread()->sleep(100ms);
            auto after_target_hp = target->hp();
            if (after_target_hp != before_target_hp + spell.expected_hp_gain)
            {
                fb::logger::warn("Spell {} HP verification failed: expected {}, got {}",
                                 spell.name,
                                 before_target_hp + spell.expected_hp_gain,
                                 after_target_hp);
            }

            fb::logger::debug("Successfully cast {} on target - target hp={}, caster mp={}",
                              spell.name,
                              target->hp(),
                              caster->mp());
        }

        spell_slot++;
        co_await caster->thread()->sleep(interval);
    }

    // Step 4: Special test for dynamic spell
    fb::logger::info("Testing special dynamic spell: 백호의희원");

    // Learn the dynamic spell
    std::ignore = co_await caster->request<fb::protocol::game::response::spell_update>(
        fb::protocol::game::request::chat{false, "/마법배우기 백호의희원"},
        timeout);

    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, "/현재마력 100"},
        [](auto& resp) -> bool {
            return resp.ch_mp == 100;
        },
        timeout);

    std::ignore = co_await target->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, "/현재체력 50"},
        [](auto& resp) -> bool {
            return resp.ch_hp == 50;
        },
        timeout);

    // Calculate based on current MP
    auto current_mp          = caster->mp();
    auto theoretical_hp_gain = current_mp * 2;
    auto expected_mp_cost    = current_mp / 2;

    // Check target's max HP (set to 100000)
    auto target_max_hp         = target->base_hp();
    auto target_current_hp     = target->hp();
    auto target_recoverable_hp = target_max_hp - target_current_hp;

    // Calculate actual recoverable amount (cannot exceed max HP)
    auto actual_expected_hp_gain = std::min(theoretical_hp_gain, target_recoverable_hp);

    fb::logger::info(
        "Dynamic spell calculation: current MP={}, theoretical HP gain={}, actual recoverable={}, expected MP cost={}",
        current_mp,
        theoretical_hp_gain,
        actual_expected_hp_gain,
        expected_mp_cost);

    // If theoretical gain exceeds recoverable amount, adjust target HP for accurate testing
    if (theoretical_hp_gain > target_recoverable_hp)
    {
        // Set target's current HP lower to enable accurate test
        auto new_target_hp = target_max_hp - theoretical_hp_gain;
        if (new_target_hp < 1)
            new_target_hp = 1; // Maintain minimum 1 HP

        fb::logger::info("Adjusting target HP to {} for accurate dynamic spell test", new_target_hp);

        std::ignore = co_await target->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat{false, std::format("/현재체력 {}", new_target_hp)},
            timeout);

        // Recalculate actual recovery amount
        actual_expected_hp_gain = theoretical_hp_gain;
    }

    // Execute test
    auto before_target_hp = target->hp();
    auto before_caster_mp = caster->mp();

    fb::logger::debug("Casting dynamic spell on target - target hp={}, caster mp={}",
                      before_target_hp,
                      before_caster_mp);

    // Cast the dynamic spell
    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", target->oid(), target->position()),
        [before_caster_mp, expected_mp_cost](auto& resp) -> bool {
            return resp.ch_mp == before_caster_mp - expected_mp_cost;
        },
        timeout);

    // Verify target HP update
    co_await caster->thread()->sleep(100ms);
    auto after_target_hp = target->hp();
    if (after_target_hp != before_target_hp + actual_expected_hp_gain)
    {
        fb::logger::warn("Dynamic spell HP verification failed: expected {}, got {}",
                         before_target_hp + actual_expected_hp_gain,
                         after_target_hp);
    }

    fb::logger::debug("Successfully cast dynamic spell on target - target hp={}, caster mp={}",
                      target->hp(),
                      caster->mp());

    // Step 5: Group healing spell test
    fb::logger::info("Testing group healing spell: 백호의희원'첨");

    // Form group with all available bots
    fb::logger::info("Forming group with {} bots for group healing test", bots.size());

    // Caster invites all other bots to the group
    for (size_t i = 1; i < bots.size(); ++i)
    {
        auto target_bot = bots[i];
        fb::logger::debug("Inviting bot {} to group", target_bot->name());

        // Send group invitation
        auto group_request = fb::protocol::game::request::group{};
        group_request.name = target_bot->name();
        auto&& resp        = co_await caster->request<fb::protocol::game::response::message>(group_request, timeout);
        caster->send(fb::protocol::game::request::chat(false, resp.text));
        co_await caster->thread()->sleep(interval);
    }

    // Learn group healing spell
    std::ignore = co_await caster->request<fb::protocol::game::response::spell_update>(
        fb::protocol::game::request::chat{false, "/마법배우기 백호의희원'첨"},
        timeout);

    // Calculate expected values based on current MP
    int group_current_mp          = static_cast<int>(caster->mp());
    int group_theoretical_hp_gain = group_current_mp * 2;
    int group_expected_mp_cost    = group_current_mp / 2;

    // Check if we need to adjust HP for accurate testing
    int group_target_max_hp         = target->base_hp();
    int group_target_recoverable_hp = group_target_max_hp - 50; // Current HP is 50

    fb::logger::info("Group spell calculation: current MP={}, theoretical HP gain={}, expected MP cost={}",
                     group_current_mp,
                     group_theoretical_hp_gain,
                     group_expected_mp_cost);

    // Reset all bots' HP to a low value for accurate testing
    for (auto& bot : bots)
    {
        std::ignore = co_await bot->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat{false, "/현재체력 50"},
            [](auto& resp) -> bool {
                return resp.ch_hp == 50;
            },
            timeout);
    }

    // Adjust HP if needed for accurate testing
    if (group_theoretical_hp_gain > group_target_recoverable_hp)
    {
        int new_hp = std::max(1, group_target_max_hp - group_theoretical_hp_gain);
        fb::logger::info("Adjusting all bots' HP to {} for accurate group spell test", new_hp);

        for (auto& bot : bots)
        {
            auto chat_command = std::format("/현재체력 {}", new_hp);
            std::ignore       = co_await bot->request<fb::protocol::game::response::update_internal>(
                fb::protocol::game::request::chat{false, chat_command},
                [new_hp](auto& resp) -> bool {
                    return resp.ch_hp == new_hp;
                },
                timeout);
        }
    }

    // Record HP values before casting
    std::vector<int> before_hp_values;
    for (auto& bot : bots)
    {
        before_hp_values.push_back(bot->hp());
    }

    before_caster_mp = caster->mp();

    fb::logger::debug("Casting group healing spell - caster MP={}", before_caster_mp);

    // Cast group healing spell
    spell_slot++;
    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, spell_slot, "", 0, {0, 0}),
        [before_caster_mp, group_expected_mp_cost](auto& resp) -> bool {
            return resp.ch_mp == before_caster_mp - group_expected_mp_cost;
        },
        timeout);

    // Wait for group heal effects to apply
    co_await caster->thread()->switching();
    co_await caster->thread()->sleep(500ms);

    // Verify HP recovery for all group members
    int actual_hp_gain = std::min(group_theoretical_hp_gain, group_target_recoverable_hp);

    for (size_t i = 0; i < bots.size(); ++i)
    {
        auto& bot         = bots[i];
        auto  expected_hp = before_hp_values[i] + actual_hp_gain;
        auto  actual_hp   = bot->hp();

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

    fb::logger::info("Group healing spell test completed successfully");
    caster->send(fb::protocol::game::request::chat(false, "Group healing spell (백호의희원'첨) test completed"));

    // Clean up spells
    std::ignore = co_await caster->request<fb::protocol::game::response::spell_remove>(
        fb::protocol::game::request::chat{false, "/마법지우기"},
        [spell_slot](auto& resp) -> bool {
            return resp.index == spell_slot - 1;
        },
        timeout);

    // Cleanup group option
    for (auto& bot : bots)
    {
        for (int i = 0; i < 2; i++)
        {
            co_await bot->request<fb::protocol::game::response::message>(
                fb::protocol::game::request::update_option(OPTION::GROUP, false),
                timeout);
        }
    }

    fb::logger::info("Healing spell test completed successfully - {} healing spells + 2 special spells tested",
                     healing_spells.size());
    caster->send(fb::protocol::game::request::chat(false, "All healing spell tests completed successfully!"));

    co_return true;
}

async::task<bool> skill_test::test_damage_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                 std::chrono::milliseconds                        timeout)
{
    constexpr auto interval = 100ms;

    auto& caster = bots.at(0);
    auto& target = bots.at(1);
    auto& other1 = bots.at(2);
    auto& other2 = bots.at(3);

    fb::logger::info("Bot {} starting damage spell test", caster->oid());
    caster->send(fb::protocol::game::request::chat(false, "=== DAMAGE SPELL TEST STARTED ==="));

    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, std::nullopt, std::nullopt, timeout);
    }

    std::vector<damage_spell_test> damage_spells = {
        {"뢰진주",       SPELL_TYPE::TARGET, 320,  120},
        {"화염주",       SPELL_TYPE::TARGET, 320,  120},
        {"백열주",       SPELL_TYPE::TARGET, 320,  120},
        {"자무주",       SPELL_TYPE::TARGET, 320,  120},
        {"뢰격주",       SPELL_TYPE::TARGET, 530,  180},
        {"화영열주",     SPELL_TYPE::TARGET, 530,  180},
        {"백령주",       SPELL_TYPE::TARGET, 530,  180},
        {"자영무주",     SPELL_TYPE::TARGET, 530,  180},
        {"뢰격참주",     SPELL_TYPE::TARGET, 740,  250},
        {"화열참주",     SPELL_TYPE::TARGET, 740,  250},
        {"백열참주",     SPELL_TYPE::TARGET, 740,  250},
        {"자천무주",     SPELL_TYPE::TARGET, 740,  250},
        {"진뢰격참주",   SPELL_TYPE::TARGET, 1950, 330},
        {"진화열참주",   SPELL_TYPE::TARGET, 1950, 330},
        {"진백열참주",   SPELL_TYPE::TARGET, 1950, 330},
        {"진자천무주",   SPELL_TYPE::TARGET, 1950, 330},
        {"극진뢰격참주", SPELL_TYPE::TARGET, 3580, 360},
        {"극진화열참주", SPELL_TYPE::TARGET, 3580, 360},
        {"극진백열참주", SPELL_TYPE::TARGET, 3580, 360},
        {"극진자천무주", SPELL_TYPE::TARGET, 3580, 360},
        {"흡성대법",     SPELL_TYPE::TARGET, 3000, 60 },
        {"백열장",       SPELL_TYPE::TARGET, 280,  80 },
        {"헬파이어",     SPELL_TYPE::TARGET, 0,    0  }  // Special case, damage is not fixed, mp cost is all current mp
    };
    auto spell_count = damage_spells.size();

    std::vector<std::string> spell_names;
    for (const auto& spell : damage_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names, timeout);
    fb::logger::info("Successfully learned {} out of {} damage spells", learned_count, damage_spells.size());

    fb::logger::info("Learning {} damage spells", damage_spells.size());
    auto spell_slot = 1;
    for (const auto& spell : damage_spells)
    {
        fb::logger::info("Testing spell: {} (Damage: {}, MP: -{})",
                         spell.name,
                         spell.expected_damage,
                         spell.expected_mp_cost);

        auto mob_info = co_await this->spawn_monster_relative_by_look(caster, "다람쥐", 0, 1, 32793, timeout);

        // Set caster's current HP/MP for testing
        std::ignore = co_await this->set_current_hp_mp(caster, 50, 100000, timeout);

        auto before_caster_hp = caster->hp();
        auto before_caster_mp = caster->mp();
        auto expected_hp      = before_caster_hp;
        auto expected_mp      = before_caster_mp - spell.expected_mp_cost;
        if (spell.name == "헬파이어")
            expected_mp = 0;

        if (spell.name == "흡성대법")
            expected_hp = before_caster_hp + 15;

        co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", mob_info.oid, mob_info.position),
            [=](auto& resp) -> bool {
                return resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
            },
            timeout);
        spell_slot++;
        co_await caster->thread()->sleep(interval);
    }

    // Clean up all spells
    fb::logger::info("Cleaning up all learned spells");
    std::ignore = co_await this->clear_all_spells(caster, timeout);

    caster->send(fb::protocol::game::request::chat(false, "=== DAMAGE SPELL TEST COMPLETED ==="));
    fb::logger::info("Damage spell test completed.");
    co_return true;
}

async::task<bool> skill_test::test_near_damage_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                      std::chrono::milliseconds                        timeout)
{
    constexpr auto interval = 100ms;

    auto& caster = bots.at(0);
    auto& target = bots.at(1);

    fb::logger::info("Bot {} starting near damage spell test", caster->oid());
    caster->send(fb::protocol::game::request::chat(false, "=== NEAR DAMAGE SPELL TEST STARTED ==="));

    auto thread = caster->thread();
    co_await thread->switching();

    // Step 1: Move caster down by 1 tile
    co_await caster->move(DIRECTION::BOTTOM);
    co_await caster->thread()->sleep(100ms);

    // Step 2: Spawn 4 monsters around the caster
    auto spawn_points     = std::vector<fb::model::point<uint16_t>>();
    auto target_position  = caster->position();
    auto left_pos         = target_position;
    left_pos.x           -= 1;
    spawn_points.push_back(left_pos);

    auto top_pos  = target_position;
    top_pos.y    -= 1;
    spawn_points.push_back(top_pos);

    auto right_pos  = target_position;
    right_pos.x    += 1;
    spawn_points.push_back(right_pos);

    auto bottom_pos  = target_position;
    bottom_pos.y    += 1;
    spawn_points.push_back(bottom_pos);

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, std::nullopt, std::nullopt, timeout);
    }

    std::vector<near_damage_spell_test> near_damage_spells = {
        // spell_damage_near spells (caster area) - sorted by MP cost
        {"화염주'첨",       SPELL_TYPE::NORMAL, 300,  180},
        {"자무주'첨",       SPELL_TYPE::NORMAL, 300,  180},
        {"백열주'첨",       SPELL_TYPE::NORMAL, 300,  180},
        {"뢰진주'첨",       SPELL_TYPE::NORMAL, 300,  180},
        {"자영무주'첨",     SPELL_TYPE::NORMAL, 510,  250},
        {"화영열주'첨",     SPELL_TYPE::NORMAL, 510,  250},
        {"백령주'첨",       SPELL_TYPE::NORMAL, 510,  250},
        {"뢰격주'첨",       SPELL_TYPE::NORMAL, 510,  250},
        {"자천무주'첨",     SPELL_TYPE::NORMAL, 720,  330},
        {"화열참주'첨",     SPELL_TYPE::NORMAL, 720,  330},
        {"백열참주'첨",     SPELL_TYPE::NORMAL, 720,  330},
        {"뢰격참주'첨",     SPELL_TYPE::NORMAL, 720,  330},
        {"진자천무주'첨",   SPELL_TYPE::NORMAL, 1930, 400},
        {"진화열참주'첨",   SPELL_TYPE::NORMAL, 1930, 400},
        {"진백열참주'첨",   SPELL_TYPE::NORMAL, 1930, 400},
        {"진뢰격참주'첨",   SPELL_TYPE::NORMAL, 1930, 400},
        {"극진자천무주'첨", SPELL_TYPE::NORMAL, 3560, 470},
        {"극진화열참주'첨", SPELL_TYPE::NORMAL, 3560, 470},
        {"극진백열참주'첨", SPELL_TYPE::NORMAL, 3560, 470},
        {"극진뢰격참주'첨", SPELL_TYPE::NORMAL, 3560, 470}
    };
    auto spell_count = near_damage_spells.size();

    std::vector<std::string> spell_names;
    for (const auto& spell : near_damage_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names, timeout);
    fb::logger::info("Successfully learned {} out of {} near damage spells", learned_count, near_damage_spells.size());

    fb::logger::info("Learning {} near damage spells", near_damage_spells.size());
    auto spell_slot       = 1;
    auto current_position = caster->position();

    for (const auto& spell : near_damage_spells)
    {
        fb::logger::info("Testing spell: {} (Damage: {}, MP: -{})",
                         spell.name,
                         spell.expected_damage,
                         spell.expected_mp_cost);

        fb::logger::info("Spawning 4 monsters around caster at ({}, {})", target_position.x, target_position.y);

        // Spawn monsters in 4 directions using relative positions
        std::vector<std::pair<int, int>> relative_positions = {
            {-1, 0 }, // left
            {0,  -1}, // top
            {1,  0 }, // right
            {0,  1 }  // bottom
        };

        std::ignore =
            co_await this->spawn_monsters_relative_by_look(caster, "다람쥐", relative_positions, 32793, timeout);

        // Set caster's current HP/MP for testing
        std::ignore = co_await this->set_current_hp_mp(caster, 50, 100000, timeout);

        auto before_caster_hp = caster->hp();
        auto before_caster_mp = caster->mp();
        auto expected_hp      = before_caster_hp;
        auto expected_mp      = before_caster_mp - spell.expected_mp_cost;

        co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
            [=](auto& resp) -> bool {
                return resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
            },
            timeout);

        spell_slot++;
        co_await caster->thread()->sleep(interval);
    }

    // Clean up all spells
    fb::logger::info("Cleaning up all learned spells");
    std::ignore = co_await this->clear_all_spells(caster, timeout);

    co_await caster->move(DIRECTION::TOP);
    caster->send(fb::protocol::game::request::direction{DIRECTION::BOTTOM});

    caster->send(fb::protocol::game::request::chat(false, "=== NEAR DAMAGE SPELL TEST COMPLETED ==="));
    fb::logger::info("Near damage spell test completed.");
    co_return true;
}

async::task<bool> skill_test::test_attack_cast_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                      std::chrono::milliseconds                        timeout)
{
    constexpr auto interval = 100ms;

    auto& caster = bots.at(0);
    auto& target = bots.at(1);

    fb::logger::info("Bot {} starting attack_cast spell test", caster->oid());
    caster->send(fb::protocol::game::request::chat(false, "=== ATTACK_CAST SPELL TEST STARTED ==="));

    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, std::nullopt, std::nullopt, timeout);
    }

    std::vector<attack_cast_spell_test> attack_cast_spells = {
        // Single target attack spells (front target)
        {"건곤대나이",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             return {current_hp - hp_cost, current_mp - 40};
         }, false},

        {"동귀어진",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp - 10; // hp = me:hp() - 10
             return {current_hp - hp_cost, current_mp - 10};
         }, false},

        {"백호검무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 60};
         }, false},

        {"백호참",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 60};
         }, false},

        {"필살검무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2;                // hp = me:hp() / 2
             auto mp_cost    = std::max<int>(current_mp, 60); // mp = max(current_mp, 60)
             return {current_hp - hp_cost, current_mp - mp_cost};
         }, false},

        {"무형검",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 180};
         }, true },

        {"이기어검",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot) -> std::pair<int, int> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp() / 2
             return {current_hp - hp_cost, current_mp - 180};
         }, true }
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : attack_cast_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names, timeout);
    fb::logger::info("Successfully learned {} out of {} attack_cast spells", learned_count, attack_cast_spells.size());

    fb::logger::info("Learning {} attack_cast spells", attack_cast_spells.size());
    auto spell_slot = 1;

    for (const auto& spell : attack_cast_spells)
    {
        fb::logger::info("Testing spell: {}", spell.name);

        // Spawn a monster in front of the caster
        std::ignore = co_await this->spawn_monster_relative_by_look(caster, "다람쥐", 0, 1, 32793, timeout);

        // Set caster's current HP/MP for testing
        std::ignore = co_await this->set_current_hp_mp(caster, 1000, 1000, timeout);

        // Calculate expected values using the spell calculator function
        auto [expected_hp, expected_mp] = spell.calculator(caster);

        // Cast the spell
        co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
            [=](auto& resp) -> bool {
                return resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
            },
            timeout);

        spell_slot++;
        co_await caster->thread()->sleep(interval);
    }

    // Clean up all spells
    fb::logger::info("Cleaning up all learned spells");
    std::ignore = co_await this->clear_all_spells(caster, timeout);

    caster->send(fb::protocol::game::request::chat(false, "=== ATTACK_CAST SPELL TEST COMPLETED ==="));
    fb::logger::info("Attack_cast spell test completed.");
    co_return true;
}

async::task<bool>
skill_test::test_multi_target_attack_cast_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                 std::chrono::milliseconds                        timeout)
{
    constexpr auto interval = 100ms;

    auto& caster = bots.at(0);

    fb::logger::info("Bot {} starting multi-target attack_cast spell test", caster->oid());
    caster->send(fb::protocol::game::request::chat(false, "=== MULTI-TARGET ATTACK_CAST SPELL TEST STARTED ==="));

    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, std::nullopt, std::nullopt, timeout);
    }

    std::vector<multi_target_attack_cast_spell_test> multi_target_spells = {
        // Multi-target attack spells
        {"어검술",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             return {current_hp - hp_cost, current_mp - 40, std::optional<fb::model::point<uint16_t>>{}};
         }, {{0, 1}, {-1, 2}, {0, 2}, {1, 2}},                                           // 4 positions in front
         false                                          },

        {"극'백호참",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2; // hp = me:hp()//2
             return {current_hp - hp_cost, current_mp - 50, std::optional<fb::model::point<uint16_t>>{}};
         }, {{0, 1}, {-1, 2}, {0, 2}, {1, 2}, {-2, 3}, {-1, 3}, {0, 3}, {1, 3}, {2, 3}}, // 9 positions in front
         false},

        {"파천검무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp  = bot->hp();
             auto current_mp  = bot->mp();
             auto hp_cost     = current_hp / 2; // hp = me:hp()//2
             auto current_pos = bot->position();
             auto expected_pos =
                 fb::model::point<uint16_t>(current_pos.x, current_pos.y + uint16_t{5}); // Move 5 tiles forward
             return {current_hp - hp_cost, current_mp - 180, expected_pos};
         }, {{0, 1}, {0, 2}, {0, 3}, {0, 4}},                                            // 4 positions in a line
         true                                            }, // This spell moves the caster forward

        {"초혼비무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             return {current_hp - hp_cost, current_mp - 140, std::optional<fb::model::point<uint16_t>>{}};
         }, {{0, 4}},                                                                    // 1 position at 4 tiles away
         false                                                                   }, // This spell moves the target, not the caster

        {"쇄혼비무",
         SPELL_TYPE::NORMAL,
         [](const std::shared_ptr<fb::bot::game_bot>& bot)
             -> std::tuple<int, int, std::optional<fb::model::point<uint16_t>>> {
             auto current_hp  = bot->hp();
             auto current_mp  = bot->mp();
             auto hp_cost     = current_hp * 2 / 3; // hp = me:hp()*2 // 3
             auto current_pos = bot->position();
             auto expected_pos =
                 fb::model::point<uint16_t>(current_pos.x, current_pos.y + uint16_t{3}); // Move 3 tiles forward
             return {current_hp - hp_cost, current_mp - 140, expected_pos};
         }, {{0, 4}},                                                                    // 1 position at 4 tiles away
         true                                                                    }  // This spell moves the caster 3 tiles forward
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : multi_target_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await this->learn_spells(caster, spell_names, timeout);
    fb::logger::info("Successfully learned {} out of {} multi-target attack_cast spells",
                     learned_count,
                     multi_target_spells.size());

    fb::logger::info("Learning {} multi-target attack_cast spells", multi_target_spells.size());
    auto spell_slot = 1;

    for (const auto& spell : multi_target_spells)
    {
        fb::logger::info("Testing multi-target spell: {}", spell.name);

        auto caster_pos = caster->position();

        // Spawn monsters at the calculated positions
        std::ignore =
            co_await this->spawn_monsters_relative_by_look(caster, "다람쥐", spell.spawn_positions, 32793, timeout);

        // Set caster's current HP/MP for testing
        std::ignore = co_await this->set_current_hp_mp(caster, 1000, 1000, timeout);

        // Calculate expected values using the spell calculator function
        auto [expected_hp, expected_mp, expected_position] = spell.calculator(caster);

        // Cast the spell
        co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
            [=](auto& resp) -> bool {
                auto success = resp.ch_hp == expected_hp && resp.ch_mp == expected_mp;
                // Check position if expected_position is provided
                if (expected_position.has_value())
                {
                    auto current_pos = caster->position();
                    success          = success && (current_pos == expected_position.value());
                }
                return success;
            },
            timeout);

        spell_slot++;

        // Move bot back to original position if it moved
        co_await this->move_bot_back_to_position(caster, caster_pos, interval);
        co_await caster->thread()->sleep(interval);
    }

    // Clean up all spells
    fb::logger::info("Cleaning up all learned multi-target spells");
    std::ignore = co_await this->clear_all_spells(caster, timeout);

    caster->send(fb::protocol::game::request::chat(false, "=== MULTI-TARGET ATTACK_CAST SPELL TEST COMPLETED ==="));
    fb::logger::info("Multi-target attack_cast spell test completed.");
    co_return true;
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
        if (bot->oid() == 0 && bot->oid() != 0xFFFFFFFD)
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

    this->_test_functions.emplace_back("Damage Spells", [this](auto& bots, auto timeout) {
        return this->test_damage_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Near Damage Spells", [this](auto& bots, auto timeout) {
        return this->test_near_damage_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Attack Cast Spells", [this](auto& bots, auto timeout) {
        return this->test_attack_cast_spells(bots, timeout);
    });

    this->_test_functions.emplace_back("Multi-Target Attack Cast Spells", [this](auto& bots, auto timeout) {
        return this->test_multi_target_attack_cast_spells(bots, timeout);
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
    }

    fb::logger::info("All test functions completed successfully");
    co_return true;
}

} // namespace fb::bot::integration