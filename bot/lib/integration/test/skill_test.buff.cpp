#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_buff_debuff_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                      std::chrono::milliseconds                        timeout)
{
    constexpr auto interval = 100ms;

    if (bots.size() < 2)
        co_return false;

    auto& caster = bots.at(0);
    auto& target = bots.at(1);

    fb::logger::info("Bot {} starting buff/debuff spell test", caster->oid());
    caster->send(fb::protocol::game::request::chat(false, "=== BUFF/DEBUFF SPELL TEST STARTED ==="));

    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await this->setup_bot_stats(bot, 100000, 100000, std::nullopt, std::nullopt, timeout);
    }

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
        {"진백호령",
         SPELL_TYPE::NORMAL,
         true,  30,
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
        {"파력무참",
         SPELL_TYPE::NORMAL,
         true,  0,
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
        {"이중공격",
         SPELL_TYPE::NORMAL,
         true,  60,
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
        {"오중공격",
         SPELL_TYPE::NORMAL,
         true,  200,
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
        {"신의축복",
         SPELL_TYPE::NORMAL,
         true,  30,
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
        {"분신",
         SPELL_TYPE::NORMAL,
         true,  0,
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
        {"보호",
         SPELL_TYPE::TARGET,
         false, 30,
         OBJECT_TYPE::UNKNOWN,
         [](const auto& caster, const auto& target) -> async::task<bool> {
             co_return true;
         }},
        {"무장",
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

    auto learned_count = co_await this->learn_spells(caster, spell_names, timeout);
    fb::logger::info("Successfully learned {} out of {} buff/debuff spells", learned_count, buff_debuff_spells.size());

    fb::logger::info("Testing {} buff/debuff spells", buff_debuff_spells.size());
    auto spell_slot = 1;

    for (const auto& spell : buff_debuff_spells)
    {
        if (spell.target_type != OBJECT_TYPE::UNKNOWN) // TODO: Add tests for spells that only work on monsters
        {
            spell_slot++;
            continue;
        }

        fb::logger::info("Testing buff/debuff spell: {}", spell.name);

        // Set caster's current HP/MP for testing
        std::ignore = co_await this->set_current_hp_mp(caster, 1000, 1000, timeout);

        // Set target's current HP/MP for testing
        std::ignore = co_await this->set_current_hp_mp(target, 1000, 1000, timeout);

        auto before_caster_mp   = caster->mp();
        auto expected_caster_mp = before_caster_mp - spell.expected_mp_cost;

        // Determine the actual target for the spell
        auto& actual_target   = spell.is_self_cast ? caster : target;
        auto  target_oid      = actual_target->oid();
        auto  target_position = actual_target->position();

        // Cast the spell
        caster->send(fb::protocol::game::request::chat(false, spell.name));
        while (true)
        {
            auto&& resp = co_await caster->request<fb::protocol::game::response::message>(
                fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", target_oid, target_position),
                timeout);

            if (resp.text == std::format("{} 외웠습니다.", name_with(spell.name)))
                break;

            caster->send(fb::protocol::game::request::chat(false, resp.text));
            co_await caster->thread()->sleep(interval);
            std::ignore = co_await this->set_current_hp_mp(target, 1000, 1000, timeout);
        }

        if (caster->mp() != expected_caster_mp)
            throw std::runtime_error(std::format("MP cost mismatch for '{}'", spell.name));

        // Wait for spell effect to propagate
        co_await caster->thread()->sleep(interval);

        // Request self_info from target to check buff/debuff
        co_await actual_target->request<fb::protocol::game::response::spell_buff>(
            fb::protocol::game::request::self_info(),
            [&](auto& resp) -> bool {
                return resp.name == spell.name;
            },
            timeout);

        fb::logger::info("Buff/Debuff '{}' successfully applied and verified", spell.name);

        // Run additional condition check
        auto condition_result = co_await spell.condition_check(caster, actual_target);
        if (condition_result)
        {
            fb::logger::info("Additional condition check passed for '{}'", spell.name);
        }
        else
        {
            fb::logger::warn("Additional condition check failed for '{}'", spell.name);
        }

        actual_target->send(fb::protocol::game::request::chat(false, "/버프해제"));

        spell_slot++;
        co_await caster->thread()->sleep(interval);
    }

    caster->send(fb::protocol::game::request::chat(false, "=== BUFF/DEBUFF SPELL TEST COMPLETED ==="));
    fb::logger::info("Buff/debuff spell test completed.");
    co_return true;
}
