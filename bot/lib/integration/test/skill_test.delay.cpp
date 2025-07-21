#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_skill_cooldown_delays(std::shared_ptr<fb::bot::game_bot> caster)
{
    fb::logger::debug("Bot {} starting skill cooldown delay test", caster->oid());
    caster->chat("=== SKILL COOLDOWN DELAY TEST STARTED ===");

    // 1. Set MP to 1000, HP to 10000000
    std::ignore = co_await caster->setup_bot_stats(10000000, 1000, 10000000, 1000, DEFAULT_TIMEOUT);
    fb::logger::debug("Set HP to 10000000, MP to 1000");

    // 2. Learn 헬파이어 spell and cast on self (response type: update_internal, ch_mp = 0)
    auto spell_slot = co_await caster->learn_spell("헬파이어", DEFAULT_TIMEOUT);
    if (spell_slot == 0xFF)
    {
        fb::logger::fatal("Failed to learn 헬파이어 spell");
        co_return false;
    }

    // Cast 헬파이어 on self - should consume all MP
    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", caster->oid(), caster->position()),
        [](auto& resp) -> bool {
            return resp.ch_mp == 0; // MP should be 0 after casting
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("헬파이어 cast successful, MP consumed to 0");

    // 3. Restore MP to 1000
    co_await caster->change_mp(1000, DEFAULT_TIMEOUT);
    fb::logger::debug("MP restored to 1000");

    // 4. Cast again on self (response type: message. should receive cooldown message)
    auto&& resp2 = co_await caster->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", caster->oid(), caster->position()),
        [](auto& resp) -> bool {
            return resp.type == MESSAGE_TYPE::STATE &&
                   resp.text.find("초 후에 사용할 수 있습니다") != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    // Extract cooldown time from message
    auto cooldown_message = resp2.text;
    fb::logger::debug("Received cooldown message: {}", cooldown_message);

    // Parse cooldown time from message (e.g., "15초 후에 사용할 수 있습니다.")
    int first_cooldown = 0;
    if (sscanf(cooldown_message.c_str(), "%d초 후에 사용할 수 있습니다", &first_cooldown) != 1)
    {
        fb::logger::fatal("Failed to parse cooldown time from message: {}", cooldown_message);
        co_return false;
    }
    fb::logger::debug("First cooldown time: {} seconds", first_cooldown);
    caster->chat("First cooldown time: {} seconds", first_cooldown);

    // 5. Change level to 99. Create and equip '용랑제구봉'
    co_await caster->change_level(99, DEFAULT_TIMEOUT);
    co_await caster->create_item("용랑제구봉", 1, DEFAULT_TIMEOUT);

    if (co_await caster->equip(0, DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Failed to equip 용랑제구봉");
        co_return false;
    }
    fb::logger::debug("Level changed to 99, 용랑제구봉 equipped");

    // 6. Sleep for the cooldown time from step 4, then cast again (response type: message)
    auto sleep_time = std::chrono::seconds(first_cooldown);
    fb::logger::debug("Sleeping for {} seconds", first_cooldown);
    co_await caster->sleep(sleep_time);

    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", caster->oid(), caster->position()),
        [](auto& resp) -> bool {
            return resp.ch_mp == 0; // MP should be 0 after casting
        },
        DEFAULT_TIMEOUT);
    co_await caster->change_mp(1000, DEFAULT_TIMEOUT);

    auto&& resp3 = co_await caster->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, spell_slot, "", caster->oid(), caster->position()),
        [](auto& resp) -> bool {
            return resp.type == MESSAGE_TYPE::STATE &&
                   resp.text.find("초 후에 사용할 수 있습니다") != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    // 7. Should receive cooldown message, but cooldown should be 4 seconds less than step 4
    auto second_cooldown_message = resp3.text;
    fb::logger::debug("Received second cooldown message: {}", second_cooldown_message);

    int second_cooldown = 0;
    if (sscanf(second_cooldown_message.c_str(), "%d초 후에 사용할 수 있습니다", &second_cooldown) != 1)
    {
        fb::logger::fatal("Failed to parse second cooldown time from message: {}", second_cooldown_message);
        co_return false;
    }
    fb::logger::debug("Second cooldown time: {} seconds", second_cooldown);
    caster->chat("Second cooldown time: {} seconds", second_cooldown);

    // Check if second cooldown is 4 seconds less than first cooldown
    if (second_cooldown != first_cooldown - 4)
    {
        fb::logger::fatal("Cooldown reduction test failed: expected {} seconds, got {} seconds",
                          first_cooldown - 4,
                          second_cooldown);
        co_return false;
    }

    fb::logger::debug("Bot {} completed skill cooldown delay test successfully", caster->oid());
    caster->chat("=== SKILL COOLDOWN DELAY TEST COMPLETED ===");
    co_return true;
}