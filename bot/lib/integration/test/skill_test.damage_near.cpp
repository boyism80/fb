#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

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

    co_await caster->move(DIRECTION::TOP);
    caster->send(fb::protocol::game::request::direction{DIRECTION::BOTTOM});

    caster->send(fb::protocol::game::request::chat(false, "=== NEAR DAMAGE SPELL TEST COMPLETED ==="));
    fb::logger::info("Near damage spell test completed.");
    co_return true;
}
