#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

async::task<bool> skill_test::test_buff_debuff_spells(std::shared_ptr<fb::bot::game_bot> caster,
                                                      std::shared_ptr<fb::bot::game_bot> target)
{
    fb::logger::debug("Bot {} starting buff/debuff spell test", caster->oid());
    caster->chat("=== BUFF/DEBUFF SPELL TEST STARTED ===");

    // Setup all bots with max HP/MP
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);
    std::ignore = co_await target->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);

    struct buff_debuff_spell_test
    {
        std::string name;             // Spell name
        SPELL_TYPE  type;             // NORMAL (self buff) or TARGET (debuff/target buff)
        bool        is_self_cast;     // true: cast on self, false: cast on target
        int         expected_mp_cost; // Expected MP consumption
        OBJECT_TYPE target_type;      // Target object type
        std::function<async::task<bool>(const std::shared_ptr<fb::bot::game_bot>&,
                                        const std::shared_ptr<fb::bot::game_bot>&)>
            condition_check;
        // Additional effect verification function (caster, target)
    };

    auto buff_debuff_spells = std::vector<buff_debuff_spell_test>{
        // DEBUFF spells (6 total) - uses debuff_cast
        {"혼마술",
         SPELL_TYPE::TARGET,
         false, 40,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"중독",
         SPELL_TYPE::TARGET,
         false, 30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"절망",
         SPELL_TYPE::TARGET,
         false, 80,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"저주",
         SPELL_TYPE::TARGET,
         false, 30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"마비",
         SPELL_TYPE::TARGET,
         false, 80,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"노도성황",
         SPELL_TYPE::TARGET,
         false, 30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},

        // BUFF spells - Self buffs (19 total) - uses buff_cast(me, me)
        {"투명",
         SPELL_TYPE::NORMAL,
         true,  0,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"신의축복",
         SPELL_TYPE::NORMAL,
         true,  30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"백호령",
         SPELL_TYPE::NORMAL,
         true,  30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"진백호령",
         SPELL_TYPE::NORMAL,
         true,  30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"측면공격",
         SPELL_TYPE::NORMAL,
         true,  90,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"후면공격",
         SPELL_TYPE::NORMAL,
         true,  90,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"차폐",
         SPELL_TYPE::NORMAL,
         true,  100,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"전혈",
         SPELL_TYPE::NORMAL,
         true,  30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"운기",
         SPELL_TYPE::NORMAL,
         true,  30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"심안투영",
         SPELL_TYPE::NORMAL,
         true,  30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"파력무참",
         SPELL_TYPE::NORMAL,
         true,  0,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"신령지익",
         SPELL_TYPE::NORMAL,
         true,  0,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"이중공격",
         SPELL_TYPE::NORMAL,
         true,  60,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"삼중공격",
         SPELL_TYPE::NORMAL,
         true,  60,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"사중공격",
         SPELL_TYPE::NORMAL,
         true,  150,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"오중공격",
         SPELL_TYPE::NORMAL,
         true,  200,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"분신",
         SPELL_TYPE::NORMAL,
         true,  0,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"누리의힘",
         SPELL_TYPE::NORMAL,
         true,  30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"금강불체",
         SPELL_TYPE::NORMAL,
         true,  600,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},

        // BUFF spells - Target buffs (4 total) - uses buff_cast(me, you)
        {"무장",
         SPELL_TYPE::TARGET,
         false, 30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"보호",
         SPELL_TYPE::TARGET,
         false, 30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"망각",
         SPELL_TYPE::TARGET,
         false, 60,
         OBJECT_TYPE::MOB,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"대지의힘",
         SPELL_TYPE::TARGET,
         false, 30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }}
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : buff_debuff_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::debug("Successfully learned {} out of {} buff/debuff spells", learned_count, buff_debuff_spells.size());

    fb::logger::debug("Testing {} buff/debuff spells", buff_debuff_spells.size());
    auto spell_slot = 0;

    for (const auto& spell : buff_debuff_spells)
    {
        if (spell.target_type != OBJECT_TYPE::UNKNOWN) // TODO: Add tests for spells that only work on monsters
        {
            spell_slot++;
            continue;
        }

        fb::logger::debug("Testing buff/debuff spell: {}", spell.name);

        // Determine the actual target for the spell
        auto& actual_target = spell.is_self_cast ? caster : target;

        // Cast the spell
        caster->chat(std::format("Testing {}", spell.name));

        auto expected_caster_mp = uint32_t{0};
        while (true)
        {
            if (caster->map() != actual_target->map())
            {
                co_await this->sleep(DEFAULT_INTERVAL);
                continue;
            }

            auto target_oid      = actual_target->oid();
            auto target_position = actual_target->position();

            // Set caster's current HP/MP for testing
            std::ignore = co_await caster->set_current_hp_mp(10000, 10000, DEFAULT_TIMEOUT);

            // Set target's current HP/MP for testing
            std::ignore = co_await target->set_current_hp_mp(10000, 10000, DEFAULT_TIMEOUT);

            auto before_caster_mp = caster->mp();
            expected_caster_mp    = before_caster_mp - spell.expected_mp_cost;

            auto&& resp = co_await caster->request<game_resp::message>(
                game_reqs::spell_cast(spell.type, spell_slot, "", target_oid, target_position),
                DEFAULT_TIMEOUT);

            if (resp.text == std::format("{} 외웠습니다.", name_with(spell.name)))
                break;

            caster->chat(resp.text);
            actual_target->remove_buffs();
        }

        if (caster->mp() != expected_caster_mp)
            throw std::runtime_error(std::format("MP cost mismatch for '{}'", spell.name));

        // Request self_info from target to check buff/debuff
        std::ignore = co_await actual_target->request<game_resp::spell_buff>(
            game_reqs::self_info(),
            [&](auto& resp) -> bool {
                return resp.name == spell.name;
            },
            DEFAULT_TIMEOUT);

        fb::logger::debug("Buff/Debuff '{}' successfully applied and verified", spell.name);

        // Run additional condition check
        auto condition_result = co_await spell.condition_check(caster, actual_target);
        if (condition_result)
        {
            fb::logger::debug("Additional condition check passed for '{}'", spell.name);
        }
        else
        {
            fb::logger::warn("Additional condition check failed for '{}'", spell.name);
        }

        actual_target->remove_buffs();

        spell_slot++;
    }

    caster->chat("=== BUFF/DEBUFF SPELL TEST COMPLETED ===");
    fb::logger::debug("Buff/debuff spell test completed.");
    co_return true;
}
