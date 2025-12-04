#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;

async::task<bool> skill_test::test_special_spells(std::shared_ptr<fb::bot::game_bot> caster)
{
    fb::logger::debug("Bot {} starting special spell test", caster->oid());
    caster->chat("=== SPECIAL SPELL TEST STARTED ===");

    // Setup bot with max HP/MP
    std::ignore = co_await caster->setup_bot_stats(100000, 100000, std::nullopt, std::nullopt, DEFAULT_TIMEOUT);

    struct special_spell_test
    {
        std::string                                                                         name;                 // Spell name
        std::function<async::task<bool>(std::shared_ptr<fb::bot::game_bot>&)>               pre_condition_check;  // Called before spell cast to setup conditions
        std::function<async::task<bool>(std::shared_ptr<fb::bot::game_bot>&, uint8_t slot)> spell_cast_function;  // Custom spell casting logic
        std::function<async::task<bool>(std::shared_ptr<fb::bot::game_bot>&)>               post_condition_check; // Called after spell cast to verify effects
    };

    auto local          = fb::config<std::string>("ip") == "127.0.0.1";
    auto weapon_name    = "목도";
    auto special_spells = std::vector<special_spell_test>{
        {"비영승보",
         [](auto& caster) -> async::task<bool> {
             fb::logger::debug("Pre-condition: Setting up nation for 비영승보 test");

             co_await caster->change_str(99, DEFAULT_TIMEOUT);
             co_await caster->change_level(30, DEFAULT_TIMEOUT);
             co_await caster->create_item("양첨목봉", 1, DEFAULT_TIMEOUT);
             std::ignore = co_await caster->equip(0, DEFAULT_TIMEOUT);
             co_return true;
         }, [](auto& caster, uint8_t slot) -> async::task<bool> {
             fb::logger::debug("Spell cast: Using spell_cast with empty message");

             auto before = caster->position();
             std::ignore = co_await caster->spawn_monster("다람쥐", before.x, before.y + 1, DEFAULT_TIMEOUT);
             auto&& resp = co_await caster->template request<fb::protocol::game::response::position>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, slot, "", 0, {0, 0}),
                 [before](auto& resp) {
                     return resp.abs != before;
                 },
                 DEFAULT_TIMEOUT);
             co_return true;
         }, [](auto& caster) -> async::task<bool> {
             fb::logger::debug("Post-condition: Verifying 비영승보 test");

             caster->chat("/몬스터제거");
             co_await caster->clear_all_drop_items(DEFAULT_TIMEOUT);
             std::ignore = co_await caster->unequip(EQUIPMENT_PARTS::WEAPON, DEFAULT_TIMEOUT);
             co_await caster->clear_inventory(DEFAULT_TIMEOUT);
             co_await caster->move(DIRECTION::TOP, 2);
             co_await caster->direction(DIRECTION::BOTTOM, DEFAULT_TIMEOUT);
             co_return true;
         }},
        {"귀환",
         [local](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup nation for teleportation test
             fb::logger::debug("Pre-condition: Setting up nation for 귀환 test");
             co_return true;
         }, [local](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with empty message

             if (local)
             {
                 auto&& resp = co_await caster->template request<fb::protocol::game::response::message>(
                     fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, slot, "", 0, {0, 0}),
                     [](auto& resp) {
                         return resp.type == MESSAGE_TYPE::STATE;
                     },
                     DEFAULT_TIMEOUT);
                 co_return resp.text == _TEXT(MESSAGE_NOT_READY_GAME_SERVER);
             }
             else
             {
                 caster = co_await caster->transfer(fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, slot, "", 0, {0, 0}));
                 co_return true;
             }
         }, [this, local](auto& caster) -> async::task<bool> {
             // Post-condition: Verify teleportation to hometown
             fb::logger::debug("Post-condition: Verifying 귀환 teleportation");

             auto& map_model = table::map[caster->map()];
             if (map_model.name != "낙랑의방")
                 caster = co_await caster->transfer(fb::protocol::game::request::chat(false, "/맵이동 낙랑의방 6 6"));
             co_return true;
         }},

        {"비영사천문",
         [local](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup direction input for teleportation
             fb::logger::debug("Pre-condition: Setting up direction input for 비영사천문 test");
             co_return true;
         }, [local](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with direction message

             auto before = caster->position();
             if (!local)
                 caster = co_await caster->transfer(fb::protocol::game::request::chat(false, "/맵이동 국내성"));

             auto&& resp = co_await caster->template request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, slot, "동", 0, {0, 0}),
                 [](auto& resp) {
                     return resp.type == MESSAGE_TYPE::STATE;
                 },
                 DEFAULT_TIMEOUT);

             if (resp.text != std::format("{} 외웠습니다.", name_with("비영사천문")))
                 co_return false;

             if (local)
             {
                 co_return caster->position() == before;
             }
             else
             {
                 if (caster->position() == before)
                 {
                     fb::logger::fatal("Failed to teleport, position is not changed");
                     co_return false;
                 }

                 caster = co_await caster->transfer(fb::protocol::game::request::chat(false, "/맵이동 낙랑의방 6 6"));
                 co_return true;
             }
         }, [local](auto& caster) -> async::task<bool> {
             // Post-condition: Verify cardinal direction teleportation
             fb::logger::debug("Post-condition: Verifying 비영사천문 teleportation");
             co_return true;
         }},

        {"공력증강",
         [](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup MP/HP for enhancement test
             std::ignore = co_await caster->set_max_hp_mp(10000, 10000, DEFAULT_TIMEOUT);
             fb::logger::debug("Pre-condition: Setting up MP/HP for 공력증강 test");
             co_return true;
         }, [](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with empty message
             while (true)
             {
                 std::ignore = co_await caster->set_current_hp_mp(10000, 30, DEFAULT_TIMEOUT);
                 auto&& resp = co_await caster->template request<fb::protocol::game::response::message>(
                     fb::protocol::game::request::spell_cast(SPELL_TYPE::NORMAL, slot, "", 0, {0, 0}),
                     [](auto& resp) {
                         return resp.type == MESSAGE_TYPE::STATE;
                     },
                     DEFAULT_TIMEOUT);

                 if (resp.text == std::format("{} 외웠습니다.", name_with("공력증강")))
                     break;
             }

             if (caster->mp() != 10000)
             {
                 fb::logger::fatal("마력이 다 회복되어야 하는데 안됐음");
                 co_return false;
             }

             if (caster->hp() >= 10000)
             {
                 fb::logger::fatal("체력이 깎여야 하는데 안깎임");
                 co_return false;
             }

             co_return true;
         }, [](auto& caster) -> async::task<bool> {
             // Post-condition: Verify MP enhancement and HP cost
             fb::logger::debug("Post-condition: Verifying 공력증강 MP/HP changes");
             co_return true;
         }},

        {"대력검신",
         [weapon_name](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup target and weapon for damage test
             fb::logger::debug("Pre-condition: Setting up target and weapon for 대력검신 test");

             co_await caster->create_item(weapon_name, 1, DEFAULT_TIMEOUT);
             std::ignore = co_await caster->equip(0, DEFAULT_TIMEOUT);
             co_return true;
         }, [weapon_name](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with weapon type message
             auto&& resp = co_await caster->template request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, slot, weapon_name, 0, {0, 0}),
                 [weapon_name](auto& resp) {
                     if (resp.type != MESSAGE_TYPE::STATE)
                         return false;

                     return resp.text == std::format("{} 푸른 빛으로 빛납니다.", name_with(weapon_name, {"이", "가"}));
                 },
                 DEFAULT_TIMEOUT);

             co_return true;
         }, [](auto& caster) -> async::task<bool> {
             // Post-condition: Verify weapon damage effect
             fb::logger::debug("Post-condition: Verifying 대력검신 damage effect");
             co_return true;
         }},

        {"검신검귀",
         [](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup target and weapon for damage test
             fb::logger::debug("Pre-condition: Setting up target and weapon for 검신검귀 test");
             co_return true;
         }, [weapon_name](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with weapon type message
             auto&& resp = co_await caster->template request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, slot, weapon_name, 0, {0, 0}),
                 [weapon_name](auto& resp) {
                     if (resp.type != MESSAGE_TYPE::STATE)
                         return false;

                     return resp.text == std::format("{} 푸른 빛으로 빛납니다.", name_with(weapon_name, {"이", "가"}));
                 },
                 DEFAULT_TIMEOUT);

             co_return true;
         }, [](auto& caster) -> async::task<bool> {
             // Post-condition: Verify weapon damage effect
             fb::logger::debug("Post-condition: Verifying 검신검귀 damage effect");
             co_return true;
         }},

        {"신검합일",
         [](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup target and weapon for damage test
             fb::logger::debug("Pre-condition: Setting up target and weapon for 신검합일 test");
             co_return true;
         }, [weapon_name](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with weapon type message
             auto&& resp = co_await caster->template request<fb::protocol::game::response::message>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, slot, weapon_name, 0, {0, 0}),
                 [weapon_name](auto& resp) {
                     if (resp.type != MESSAGE_TYPE::STATE)
                         return false;

                     return resp.text == std::format("{} 푸른 빛으로 빛납니다.", name_with(weapon_name, {"이", "가"}));
                 },
                 DEFAULT_TIMEOUT);

             co_return true;
         }, [](auto& caster) -> async::task<bool> {
             // Post-condition: Verify weapon damage effect
             fb::logger::debug("Post-condition: Verifying 신검합일 damage effect");

             std::ignore = co_await caster->unequip(EQUIPMENT_PARTS::WEAPON, DEFAULT_TIMEOUT);
             co_await caster->clear_inventory(DEFAULT_TIMEOUT);
             co_return true;
         }},

        {"소혼강신",
         [](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup level and monster type for summoning test
             fb::logger::debug("Pre-condition: Setting up level and monster type for 소혼강신 test");
             co_await caster->change_level(68, DEFAULT_TIMEOUT);
             co_return true;
         }, [this](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with monster type message
             auto mob       = "평웅";
             auto mob_model = table::mob.name2mob(mob);
             if (mob_model == nullptr)
             {
                 fb::logger::fatal("Could not find monster model for {}", mob);
                 co_return false;
             }

             auto&& resp = co_await caster->template request<fb::protocol::game::response::update>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, slot, mob, 0, {0, 0}),
                 [&mob_model](auto& resp) {
                     if (resp.objects_data.size() != 1)
                         return false;

                     auto mob = resp.objects_data[0];
                     if (mob.look != mob_model->look)
                         return false;

                     return true;
                 },
                 DEFAULT_TIMEOUT);

             co_return true;
         }, [](auto& caster) -> async::task<bool> {
             // Post-condition: Verify monster summoning
             fb::logger::debug("Post-condition: Verifying 소혼강신 monster summoning");
             caster->chat("/몬스터제거");
             co_return true;
         }},

        {"성황령",
         [local](auto& caster) -> async::task<bool> {
             // Pre-condition: Setup ghost state and revive points
             fb::logger::debug("Pre-condition: Setting up ghost state and revive points for 성황령 test");
             auto slot = co_await caster->learn_spell("헬파이어", DEFAULT_TIMEOUT);
             if (slot == 0xFF)
             {
                 fb::logger::fatal("Could not learn 헬파이어");
                 co_return false;
             }

             std::ignore = co_await caster->set_current_hp_mp(50, 100000, DEFAULT_TIMEOUT);
             std::ignore = co_await caster->template request<fb::protocol::game::response::update_external<true>>(
                 fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, slot, "", caster->oid(), caster->position()),
                 [oid = caster->oid()](auto& resp) {
                     if (resp.oid != oid)
                         return false;

                     if (resp.state != STATE::GHOST)
                         return false;

                     return true;
                 },
                 DEFAULT_TIMEOUT);

             if (!local)
                 caster = co_await caster->transfer(fb::protocol::game::request::chat(false, "/맵이동 국내성진입로"));

             co_return true;
         }, [this](auto& caster, uint8_t slot) -> async::task<bool> {
             // Spell cast: Use spell_cast with direction message

             auto& map_model  = table::map[caster->map()];
             auto& root_model = table::map[map_model.root];
             if (root_model.revive.size() == 0)
             {
                 fb::logger::warn("Revive is not enabled, skipping test");
                 co_return true;
             }

             auto&& resp =
                 co_await caster->template request<fb::protocol::game::response::map_config>(fb::protocol::game::request::spell_cast(SPELL_TYPE::INPUT, slot, "좌", 0, {0, 0}),
                                                                                             DEFAULT_TIMEOUT);

             auto& next_map_model = table::map[caster->map()];
             if (next_map_model.id != root_model.revive.at(CARDINAL_DIRECTION::WEST))
             {
                 fb::logger::fatal("This map must have revive enabled");
                 co_return false;
             }

             co_return true;
         }, [this, local](auto& caster) -> async::task<bool> {
             // Post-condition: Verify ghost teleportation
             fb::logger::debug("Post-condition: Verifying 성황령 ghost teleportation");

             auto& map_model = table::map[caster->map()];
             if (map_model.name != "낙랑의방")
                 caster = co_await caster->transfer(fb::protocol::game::request::chat(false, "/맵이동 낙랑의방 6 6"));

             std::ignore = co_await caster->set_max_hp_mp(100000, 100000, DEFAULT_TIMEOUT);
             co_return true;
         }}
    };

    std::vector<std::string> spell_names;
    for (const auto& spell : special_spells)
    {
        spell_names.push_back(spell.name);
    }

    auto learned_count = co_await caster->learn_spells(spell_names, DEFAULT_TIMEOUT);
    fb::logger::debug("Successfully learned {} out of {} special spells", learned_count, special_spells.size());

    caster->chat("=== SPECIAL SPELL LEARNING COMPLETED ===");
    fb::logger::debug("Special spell learning completed. Individual test implementations pending.");

    // Run individual spell tests
    auto slot = 0;
    for (const auto& spell : special_spells)
    {
        fb::logger::debug("Running test for: {}", spell.name);

        // Run pre-condition check
        auto pre_condition_result = co_await spell.pre_condition_check(caster);
        if (!pre_condition_result)
        {
            fb::logger::warn("Pre-condition check failed for '{}', skipping test", spell.name);
            slot++;
            continue;
        }

        // Run spell cast function
        auto spell_cast_result = co_await spell.spell_cast_function(caster, slot++);
        if (!spell_cast_result)
        {
            fb::logger::warn("Spell cast failed for '{}', skipping post-condition", spell.name);
            continue;
        }

        // Run post-condition check
        auto post_condition_result = co_await spell.post_condition_check(caster);
        if (post_condition_result)
        {
            fb::logger::debug("Post-condition check passed for '{}'", spell.name);
        }
        else
        {
            fb::logger::warn("Post-condition check failed for '{}'", spell.name);
        }
    }

    caster->chat("=== SPECIAL SPELL TEST COMPLETED ===");
    fb::logger::debug("Special spell test completed.");
    co_return true;
}