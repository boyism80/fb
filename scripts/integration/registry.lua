-- Integration test registry
-- Execution order matches the previous C++ enqueue order in game_controller.cpp.
-- Smoke tests are prepended (not present in the original C++ list).

register_test("builtin_smoke_test")
register_test("lib_smoke_test")
register_test("request_smoke_test")
register_test("smoke_test")

register_test("movement_test")
register_test("attack_test")
register_test("skill_test")
register_test("bulletin_test")
register_test("trade_test")
register_test("communication_test")
register_test("drop_loot_test")
register_test("item_test")
register_test("item_give_test")
register_test("emotion_test")
register_test("front_info_test")
register_test("chat_interaction_test")
register_test("user_list_test")
register_test("swap_test")
register_test("throw_test")
register_test("group_test")
register_test("clan_test")
register_test("marketplace_test")

if is_local_host() == false then
    register_test("worldmap_test")
    register_test("door_test")
end
