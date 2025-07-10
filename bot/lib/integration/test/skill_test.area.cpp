#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_area_damage_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots)
{
    if (bots.size() < 2)
        co_return false;

    auto& caster = bots.at(0);

    fb::logger::info("Bot {} starting area damage spell test", caster->oid());
    caster->chat("=== AREA DAMAGE SPELL TEST STARTED ===");

    auto thread = caster->thread();
    co_await thread->switching();

    // Setup all bots with max HP/MP
    for (auto& bot : bots)
    {
        std::ignore = co_await bot->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);
    }

    struct area_damage_spell_test
    {
        std::string name;
        SPELL_TYPE  type;
        std::function<std::tuple<int, int, int>(const std::shared_ptr<fb::bot::game_bot>&)>
            calculator; // caster_hp, caster_mp, target_damage
    };

    auto area_spells = std::vector<area_damage_spell_test>{
        {"분혼경천",
         SPELL_TYPE::NORMAL,
         [](const auto& bot) {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2;       // hp = me:hp()//2
             auto mp_cost    = 200;                  // mp = 200
             auto damage     = (current_hp * 3) / 4; // damage = (me:hp()*3)//4
             return std::tuple<int, int, int>{current_hp - hp_cost, current_mp - mp_cost, damage};
         }                                        },

        {"혈겁만파",
         SPELL_TYPE::NORMAL,
         [](const auto& bot) {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2;       // hp = me:hp()//2
             auto mp_cost    = 600;                  // mp = 600
             auto damage     = (current_hp * 3) / 4; // damage = (me:hp()*3)//4
             return std::tuple<int, int, int>{current_hp - hp_cost, current_mp - mp_cost, damage};
         }                                        },

        {"폭류유성",
         SPELL_TYPE::NORMAL,
         [](const auto& bot) {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_final   = uint32_t{current_hp / 2};               // me:hp(me:hp() // 2)
             auto mp_cost    = std::max(uint32_t{30}, current_mp / 2); // mp = math.max(30, me:mp() // 2)
             auto damage     = static_cast<int>(current_hp * 0.75 +
                                            current_mp * 1.5); // damage = math.floor(me:hp()*0.75 + me:mp()*1.5)
             return std::tuple<int, int, int>{hp_final, current_mp - mp_cost, damage};
         }                                        },

        {"포효검황",
         SPELL_TYPE::NORMAL,
         [](const auto& bot) {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto hp_cost    = current_hp / 2;         // hp = me:hp()//2
             auto mp_cost    = 600;                    // mp = 600
             auto damage     = (current_hp * 11) / 10; // damage = (me:hp()*11)//10
             return std::tuple<int, int, int>{current_hp - hp_cost, current_mp - mp_cost, damage};
         }                                        },

        {"지폭지술", SPELL_TYPE::NORMAL, [](const auto& bot) {
             auto current_hp = bot->hp();
             auto current_mp = bot->mp();
             auto mp_cost    = std::max(uint32_t{current_mp}, uint32_t{30}); // mp = math.max(me:mp(), 30)
             auto damage     = (current_mp * 3) / 2;                         // damage = (me:mp()*3)//2
             return std::tuple<int, int, int>{current_hp, current_mp - mp_cost, damage};
         }}
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : area_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::info("Successfully learned {} out of {} area damage spells", learned_count, area_spells.size());

    fb::logger::info("Testing {} area damage spells", area_spells.size());
    auto spell_slot = 1;

    co_await caster->move(DIRECTION::BOTTOM, 3, DEFAULT_INTERVAL);
    co_await bots[1]->move(DIRECTION::LEFT, 2, DEFAULT_INTERVAL);
    co_await bots[1]->move(DIRECTION::BOTTOM, 3, DEFAULT_INTERVAL);
    co_await bots[2]->move(DIRECTION::LEFT, 2, DEFAULT_INTERVAL);
    co_await bots[2]->move(DIRECTION::BOTTOM, 2, DEFAULT_INTERVAL);
    co_await bots[3]->move(DIRECTION::LEFT, 2, DEFAULT_INTERVAL);
    co_await bots[3]->move(DIRECTION::BOTTOM, 4, DEFAULT_INTERVAL);
    co_await bots[3]->move(DIRECTION::LEFT, 1, DEFAULT_INTERVAL);
    co_await bots[4]->move(DIRECTION::LEFT, 3, DEFAULT_INTERVAL);
    co_await bots[4]->move(DIRECTION::BOTTOM, 3, DEFAULT_INTERVAL);
    co_await caster->thread()->sleep(500ms);
    caster->direction(DIRECTION::BOTTOM);
    bots[1]->direction(DIRECTION::BOTTOM);
    bots[2]->direction(DIRECTION::BOTTOM);
    bots[3]->direction(DIRECTION::BOTTOM);
    bots[4]->direction(DIRECTION::BOTTOM);

    for (const auto& spell : area_spells)
    {
        fb::logger::info("Testing area spell: {}", spell.name);

        co_await caster->spawn_monsters_by_look_bulk("다람쥐", 5, 32793, DEFAULT_TIMEOUT);

        // Set caster's current HP/MP for testing
        std::ignore = co_await caster->set_current_hp_mp(1000, 1000, DEFAULT_TIMEOUT);

        // Set other bots' HP for damage testing
        for (size_t i = 1; i < bots.size(); ++i)
        {
            std::ignore = co_await bots[i]->set_current_hp_mp(10000, 10000, DEFAULT_TIMEOUT);
        }

        // Calculate expected values using the spell calculator function
        auto [expected_caster_hp, expected_caster_mp, expected_damage] = spell.calculator(caster);

        // Calculate expected HP for other bots after taking damage
        std::vector<int> expected_target_hp;
        for (size_t i = 1; i < bots.size(); ++i)
        {
            auto target_hp = bots[i]->hp();
            expected_target_hp.push_back(target_hp - expected_damage);
        }

        // Cast the spell
        caster->chat(std::format("Testing {}", spell.name));
        std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(spell.type, spell_slot, "", 0, {0, 0}),
            [=](auto& resp) -> bool {
                return resp.ch_hp == expected_caster_hp && resp.ch_mp == expected_caster_mp;
            },
            DEFAULT_TIMEOUT);

        // Wait for spell effect to propagate
        co_await caster->thread()->sleep(DEFAULT_INTERVAL);

        // Verify other bots took damage
        for (size_t i = 1; i < bots.size(); ++i)
        {
            auto actual_hp   = bots[i]->hp();
            auto expected_hp = expected_target_hp[i - 1];

            if (actual_hp != expected_hp)
            {
                fb::logger::warn("Bot {} HP mismatch: expected {}, actual {}", bots[i]->oid(), expected_hp, actual_hp);
            }
            else
            {
                fb::logger::info("Bot {} HP correctly reduced to {}", bots[i]->oid(), actual_hp);
            }
        }

        spell_slot++;
        co_await caster->thread()->sleep(DEFAULT_INTERVAL);
    }

    co_await bots[4]->move(DIRECTION::TOP, 3, DEFAULT_INTERVAL);
    co_await bots[4]->move(DIRECTION::RIGHT, 3, DEFAULT_INTERVAL);
    co_await bots[3]->move(DIRECTION::RIGHT, 1, DEFAULT_INTERVAL);
    co_await bots[3]->move(DIRECTION::TOP, 4, DEFAULT_INTERVAL);
    co_await bots[3]->move(DIRECTION::RIGHT, 2, DEFAULT_INTERVAL);
    co_await bots[2]->move(DIRECTION::TOP, 2, DEFAULT_INTERVAL);
    co_await bots[2]->move(DIRECTION::RIGHT, 2, DEFAULT_INTERVAL);
    co_await bots[1]->move(DIRECTION::TOP, 3, DEFAULT_INTERVAL);
    co_await bots[1]->move(DIRECTION::RIGHT, 2, DEFAULT_INTERVAL);
    co_await caster->move(DIRECTION::TOP, 3, DEFAULT_INTERVAL);
    co_await caster->thread()->sleep(500ms);
    bots[4]->direction(DIRECTION::BOTTOM);
    bots[3]->direction(DIRECTION::BOTTOM);
    bots[2]->direction(DIRECTION::BOTTOM);
    bots[1]->direction(DIRECTION::BOTTOM);
    caster->direction(DIRECTION::BOTTOM);

    caster->chat("=== AREA DAMAGE SPELL TEST COMPLETED ===");
    fb::logger::info("Area damage spell test completed.");
    co_return true;
}
