#include <fb/bot/integration/skill_test.h>
#include <fb/bot/game_bot.h>

using namespace fb::bot::integration;
using namespace std::chrono_literals;

namespace fb::bot::integration {

struct shout_spell_test
{
    std::string  spell_name;
    int          mp_cost;
    std::string  test_message;
    MESSAGE_TYPE message_type;
};

async::task<bool> skill_test::test_shout_spells(std::vector<std::shared_ptr<fb::bot::game_bot>>& bots,
                                                std::chrono::milliseconds                        timeout)
{
    if (bots.size() < 1)
    {
        fb::logger::fatal("Shout spell test requires at least 1 bot");
        co_return false;
    }

    auto caster = bots[0];

    constexpr auto interval = 100ms;

    // Define shout spells with their test parameters
    auto shout_spells = std::vector<shout_spell_test>{
        {"사자후전사", 0, "전사 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"사자후도사", 0, "도사 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"사자후술사", 0, "술사 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"사자후도적", 0, "도적 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"세계후",     0, "세계후 테스트 메시지입니다!",      MESSAGE_TYPE::WORLD}
    };

    fb::logger::info("Testing shout spells with message input and SHOUT verification");
    caster->chat("=== SHOUT SPELL TEST STARTED ===");

    // Learn all shout spells
    auto spell_names = std::vector<std::string>{};
    for (const auto& spell : shout_spells)
    {
        fb::logger::info("Learning spell: {}", spell.spell_name);
        spell_names.push_back(spell.spell_name);
    }
    co_await this->learn_spells(caster, spell_names, timeout);

    // Test each shout spell
    auto spell_slot = 0;
    for (const auto& spell : shout_spells)
    {
        spell_slot++;
        fb::logger::info("Testing {} spell", spell.spell_name);

        // Set current hp and mp
        co_await this->set_current_hp_mp(caster, 10000, 1000, timeout);

        fb::logger::info("Testing {} with message: {}", spell.spell_name, spell.test_message);

        // Step 1: Cast shout spell with test message
        auto before_mp   = caster->mp();
        auto expected_mp = before_mp - spell.mp_cost;

        fb::logger::info("Casting {} spell with message (MP: {} -> {})", spell.spell_name, before_mp, expected_mp);

        // Cast the spell and wait for message response
        caster->chat(std::format("Testing {}", spell.spell_name));
        co_await caster->request<fb::protocol::game::response::message>(
            fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, spell_slot, spell.test_message, 0, {0, 0}),
            [&spell](auto& resp) -> bool {
                return resp.type == spell.message_type;
            },
            timeout);

        fb::logger::info("{} spell cast completed, received message response", spell.spell_name);

        // Step 2: Verify MP consumption separately
        auto after_mp = caster->mp();
        if (after_mp != expected_mp)
        {
            auto sstream = std::stringstream{};
            sstream << spell.spell_name << " MP consumption verification failed: expected " << expected_mp << ", got "
                    << after_mp;
            throw std::runtime_error(sstream.str());
        }

        fb::logger::info("{} shout spell test completed successfully", spell.spell_name);

        // Wait between spells
        co_await caster->thread()->sleep(interval);
    }

    fb::logger::info("Shout spell testing completed");
    caster->chat("=== SHOUT SPELL TEST COMPLETED ===");
    co_return true;
}

} // namespace fb::bot::integration