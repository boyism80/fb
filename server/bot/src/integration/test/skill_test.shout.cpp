#include <fb/bot/integration/skill_test.h>

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

async::task<bool> skill_test::test_shout_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    auto bots = this->get_test_bots();

    // Define shout spells with their test parameters
    auto shout_spells = std::vector<shout_spell_test>{
        {"사자후전사", 0, "전사 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"사자후도사", 0, "도사 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"사자후술사", 0, "술사 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"사자후도적", 0, "도적 사자후 테스트 메시지입니다!", MESSAGE_TYPE::SHOUT},
        {"세계후",     0, "세계후 테스트 메시지입니다!",      MESSAGE_TYPE::WORLD}
    };

    fb::logger::debug("Testing shout spells with message input and SHOUT verification");
    caster->chat("=== SHOUT SPELL TEST STARTED ===");

    // Learn all shout spells
    auto spell_names = std::vector<std::string>{};
    for (const auto& spell : shout_spells)
    {
        fb::logger::debug("Learning spell: {}", spell.spell_name);
        spell_names.push_back(spell.spell_name);
    }
    std::ignore = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);

    // Test each shout spell
    auto spell_slot = 0;
    for (const auto& spell : shout_spells)
    {
        fb::logger::debug("Testing {} spell", spell.spell_name);

        // Set current hp and mp
        std::ignore = co_await caster->set_current_hp_mp(10000, 1000, DEFAULT_TIMEOUT);

        fb::logger::debug("Testing {} with message: {}", spell.spell_name, spell.test_message);

        // Step 1: Cast shout spell with test message
        auto before_mp   = caster->mp();
        auto expected_mp = before_mp - spell.mp_cost;

        fb::logger::debug("Casting {} spell with message (MP: {} -> {})", spell.spell_name, before_mp, expected_mp);

        // Cast the spell and wait for message response
        caster->chat(std::format("Testing {}", spell.spell_name));
        std::ignore = co_await caster->request<fb::protocol::game::response::message>(
            fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, spell_slot++, spell.test_message, 0, {0, 0}),
            [&spell](auto& resp) -> bool {
                return resp.type == spell.message_type;
            },
            DEFAULT_TIMEOUT);

        fb::logger::debug("{} spell cast completed, received message response", spell.spell_name);

        // Step 2: Verify MP consumption separately
        auto after_mp = caster->mp();
        if (after_mp != expected_mp)
        {
            auto sstream = std::stringstream{};
            sstream << spell.spell_name << " MP consumption verification failed: expected " << expected_mp << ", got "
                    << after_mp;
            throw std::runtime_error(sstream.str());
        }

        fb::logger::debug("{} shout spell test completed successfully", spell.spell_name);
    }

    fb::logger::debug("Shout spell testing completed");
    caster->chat("=== SHOUT SPELL TEST COMPLETED ===");
    co_return true;
}

} // namespace fb::bot::integration