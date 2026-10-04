-- Integration test registry
-- Phase A: parallel (default). Phase B: serial after Phase A drains.
-- label (a name or a list of names) selects the tests run by bot.exe --label <label>.
-- Tests without a label are "regular", which also runs when --label is omitted.
--   regular   the regular suite
--   script    NPC script tests
--   explorer  NPC dialog explorer, about 9 minutes with 16 bots

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
register_test("instance_map_test", { extra_slot = true })
register_test("chat_interaction_test")
register_test("user_list_test")
register_test("swap_test")
register_test("throw_test")
register_test("group_test")
register_test("matchmaking_test")
register_test("clan_test")
register_test("marriage_test", { serial = true })
register_test("marketplace_test", { serial = true })
register_test("storage_box_test")
register_test("worldmap_test")
register_test("door_test")
register_test("rabbit_quest_test", { label = { "regular", "script" } })
register_test("mob_parts_test")
register_test("resist_test")

register_test("quest_clear_shield_test", { label = "script" })
register_test("quest_lighthouse_test", { label = "script" })
register_test("quest_reward_capacity_test", { label = "script" })
register_test("quest_hwangbiyeon_test", { label = "script" })

register_test("npc_explorer_test", { label = "explorer" })
