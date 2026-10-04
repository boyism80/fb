-- QUEST_CLEAR_SHIELD (209): 우선녀(npc/77) -> 랑구륜(npc/2) -> 우선녀.
-- Expected values come from the two NPC scripts: each step checks the dialog text,
-- the quest step and the inventory on the server, including the refusal paths.
local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_ID        = 209
local NPC_WOOSEONNYEO = "우선녀"
local NPC_RANGGURYUN  = "랑구륜"

local contains, check = qd.contains, qd.check
local click, press, next_dialog, select_option = qd.click, qd.press, qd.next_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "clear_shield: " .. string.format(fmt, ...))
    return false
end

local function open_pure_water(bot, expect)
    return qd.open_pursuit(bot, NPC_RANGGURYUN, "순수한물", expect)
end

test_suite {
    name      = "Quest Clear Shield Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        check(bot, "set_quest", "reset", QUEST_ID, "none")
        bot:clear_inventory()
        qd.remove_npc(bot)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "init", QUEST_ID, "none") == false then
                return fail("could not reset the quest")
            end
            bot:clear_inventory()

            -- Refusing the request must not start the quest.
            local dlg = click(bot, NPC_WOOSEONNYEO, contains("용궁을 드나드는"))
            dlg = dlg and next_dialog(bot, contains("랑구륜이 알고"))
            dlg = dlg and next_dialog(bot, contains("가서 좀 알아봐"))
            dlg = dlg and select_option(bot, dlg, "별로..내키지가 않아서...", contains("내키지가"))
            if dlg == nil then
                return fail("refusal dialog flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "refused", QUEST_ID, "none") == false then
                return fail("refusing started the quest")
            end

            -- Accepting starts the quest at step 1.
            dlg = click(bot, NPC_WOOSEONNYEO, contains("용궁을 드나드는"))
            dlg = dlg and next_dialog(bot, contains("랑구륜이 알고"))
            dlg = dlg and next_dialog(bot, contains("가서 좀 알아봐"))
            dlg = dlg and select_option(bot, dlg, "물론입니다.", contains("고마워요"))
            if dlg == nil then
                return fail("accept dialog flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "accepted", QUEST_ID, "1") == false then
                return fail("accepting did not set step 1")
            end

            -- 우선녀 only points to 랑구륜 while the quest is at step 1.
            if click(bot, NPC_WOOSEONNYEO, contains("랑구륜에게 가보시면")) == nil then
                return fail("우선녀 step 1 hint missing")
            end
            press(bot, "NEXT")

            -- 랑구륜 asks for 3 홍옥 (step 2).
            dlg = open_pure_water(bot, function(p) return p.type == "list" end)
            dlg = dlg and select_option(bot, dlg, "물을 정화시키는 방법을 아시나요?", contains("대가가 필요"))
            dlg = dlg and select_option(bot, dlg, "무슨 대가인가요?", contains("홍옥 3개만"))
            dlg = dlg and select_option(bot, dlg, "예. 알겠습니다.", contains("꼭 홍옥으로"))
            if dlg == nil then
                return fail("랑구륜 request dialog flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "asked_apples", QUEST_ID, "2") == false then
                return fail("랑구륜 did not set step 2")
            end

            -- Without enough 홍옥 nothing changes.
            if open_pure_water(bot, contains("아직 홍옥 3개를")) == nil then
                return fail("랑구륜 accepted zero 홍옥")
            end
            press(bot, "NEXT")
            bot:create_item("홍옥", 2)
            if open_pure_water(bot, contains("아직 홍옥 3개를")) == nil then
                return fail("랑구륜 accepted two 홍옥")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "short_apples", QUEST_ID, "2") == false
                or check(bot, "verify_items", "short_apples_items", "홍옥=2,정화비서=0") == false then
                return fail("state changed without enough 홍옥")
            end

            -- 3 홍옥 are exchanged for 정화비서 (step 3).
            -- create_item waits for the exact stack count, so build the stack in one call.
            bot:clear_inventory()
            bot:create_item("홍옥", 3)
            dlg = open_pure_water(bot, contains("홍옥을 가져오셨군요"))
            dlg = dlg and next_dialog(bot, contains("우물우물"))
            dlg = dlg and next_dialog(bot, contains("정화비서를 가져다"))
            if dlg == nil then
                return fail("홍옥 exchange dialog flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "got_book", QUEST_ID, "3") == false
                or check(bot, "verify_items", "got_book_items", "홍옥=0,정화비서=1") == false then
                return fail("홍옥 were not exchanged for 정화비서")
            end

            -- Quitting 우선녀's dialog midway keeps 정화비서 and step 3.
            if click(bot, NPC_WOOSEONNYEO, contains("비법을 알아오셨네요")) == nil then
                return fail("우선녀 step 3 dialog missing")
            end
            press(bot, "QUIT")
            if check(bot, "verify_quest", "quit_book", QUEST_ID, "3") == false
                or check(bot, "verify_items", "quit_book_items", "정화비서=1") == false then
                return fail("quitting the dialog changed the state")
            end

            -- Without 정화비서 우선녀 does not advance.
            bot:clear_inventory()
            if click(bot, NPC_WOOSEONNYEO, contains("아직 물을 정화시키는 법을")) == nil then
                return fail("우선녀 advanced without 정화비서")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "no_book", QUEST_ID, "3") == false then
                return fail("step changed without 정화비서")
            end

            -- Handing over 정화비서 sets step 4 and removes the book.
            bot:create_item("정화비서", 1)
            dlg = click(bot, NPC_WOOSEONNYEO, contains("비법을 알아오셨네요"))
            dlg = dlg and next_dialog(bot, contains("간단한 것을"))
            dlg = dlg and next_dialog(bot, contains("숯의정화 3조각만 가져다"))
            dlg = dlg and next_dialog(bot, contains("불의 힘이 깃든"))
            if dlg == nil then
                return fail("정화비서 hand-over dialog flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "gave_book", QUEST_ID, "4") == false
                or check(bot, "verify_items", "gave_book_items", "정화비서=0") == false then
                return fail("정화비서 hand-over did not set step 4 and take the book")
            end

            -- Without 숯의정화 nothing is exchanged.
            if click(bot, NPC_WOOSEONNYEO, contains("숯의정화 3조각만 구해주세요")) == nil then
                return fail("우선녀 accepted zero 숯의정화")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "no_charcoal", QUEST_ID, "4") == false
                or check(bot, "verify_items", "no_charcoal_items", "정화의방패=0") == false then
                return fail("state changed without 숯의정화")
            end

            -- 3 숯의정화 complete the quest and give 정화의방패.
            bot:create_item("숯의정화", 3)
            dlg = click(bot, NPC_WOOSEONNYEO, contains("다시 깨끗함을"))
            dlg = dlg and next_dialog(bot, contains("감사의 뜻으로"))
            if dlg == nil then
                return fail("completion dialog flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "completed", QUEST_ID, "5:done") == false
                or check(bot, "verify_items", "completed_items", "숯의정화=0,정화의방패=1") == false then
                return fail("completion did not finish the quest or give 정화의방패")
            end

            -- After completion 우선녀 only thanks the player and gives nothing again.
            bot:create_item("숯의정화", 3)
            if click(bot, NPC_WOOSEONNYEO, contains("덕분에 용궁의 물이")) == nil then
                return fail("우선녀 completed dialog missing")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "after_done", QUEST_ID, "5:done") == false
                or check(bot, "verify_items", "after_done_items", "숯의정화=3,정화의방패=1") == false then
                return fail("우선녀 rewarded again after completion")
            end

            -- 랑구륜 no longer offers the quest dialog.
            bot:create_item("홍옥", 3)
            if open_pure_water(bot, contains("홍옥의 그 광채")) == nil then
                return fail("랑구륜 still runs the quest after completion")
            end
            press(bot, "NEXT")
            if check(bot, "verify_items", "rang_after_done_items", "홍옥=3,정화비서=0") == false then
                return fail("랑구륜 exchanged 홍옥 after completion")
            end
            return true
        end,
    },
}
