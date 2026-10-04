-- QUEST_LIGHTHOUSE (190): 진백랑(npc/174) -> 진진(npc/118) -> 퉁퉁대감(npc/120) -> 통통대감(npc/122)
-- -> 탕탕대감(npc/121) -> 준준(npc/115) -> 진백랑.
-- Expected values come from the NPC scripts: every hand-over checks the dialog text, the quest
-- steps of the chain and the sub-quests, and the inventory on the server.
local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_LIGHTHOUSE = 190
local QUEST_TANGTANG   = 183
local QUEST_JUNJUN     = 184
local QUEST_JINJIN     = 185
local QUEST_TUNGTUNG   = 186
local QUEST_TONGTONG   = 187
local ALL_QUESTS       = { QUEST_LIGHTHOUSE, QUEST_TANGTANG, QUEST_JUNJUN, QUEST_JINJIN, QUEST_TUNGTUNG, QUEST_TONGTONG }

local INVENTORY_CAPACITY = 52

local contains, is_list, check = qd.contains, qd.is_list, qd.check
local click, open_pursuit, press, choose = qd.click, qd.open_pursuit, qd.press, qd.choose
local next_dialog, prev_dialog, select_option = qd.next_dialog, qd.prev_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "lighthouse: " .. string.format(fmt, ...))
    return false
end

local function reset_quests(bot, step)
    for _, id in ipairs(ALL_QUESTS) do
        if check(bot, "set_quest", step, id, "none") == false then
            return false
        end
    end
    return true
end

local function accept_jinbaekrang(bot)
    local dlg = click(bot, "진백랑", contains("대륙의 기운이"))
    dlg = dlg and next_dialog(bot, is_list)
    dlg = dlg and select_option(bot, dlg, "네, 도와드리지요.", contains("서복이란"))
    dlg = dlg and next_dialog(bot, contains("죽었다는 사실은"))
    dlg = dlg and prev_dialog(bot, contains("서복이란"))
    dlg = dlg and next_dialog(bot, contains("죽었다는 사실은"))
    dlg = dlg and next_dialog(bot, contains("아홉장과 겉표지를"))
    if dlg == nil then
        return false
    end
    press(bot, "NEXT")
    return true
end

local function deliver_totem(bot, totem)
    local dlg = open_pursuit(bot, "준준", "문화재보호공무원", is_list)
    dlg = dlg and select_option(bot, dlg, "토템을 하나 찾아왔습니다", is_list)
    dlg = dlg and select_option(bot, dlg, totem, contains("수고했네. 계속 힘내주게."))
    if dlg == nil then
        return false
    end
    press(bot, "NEXT")
    return true
end

local function claim_totem_reward(bot, expect)
    local dlg = open_pursuit(bot, "준준", "문화재보호공무원", is_list)
    dlg = dlg and select_option(bot, dlg, "보상을 요구한다.", expect)
    if dlg == nil then
        return false
    end
    press(bot, "NEXT")
    return true
end

test_suite {
    name      = "Quest Lighthouse Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        reset_quests(bot, "reset")
        bot:clear_inventory()
        qd.remove_npc(bot)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if reset_quests(bot, "init") == false then
                return fail("could not reset the quests")
            end
            bot:clear_inventory()

            -- The chain NPCs stay silent before the chain reaches them.
            if click(bot, "퉁퉁대감", contains("....")) == nil then
                return fail("퉁퉁대감 answered before the chain started")
            end
            press(bot, "NEXT")

            -- 진백랑: refusing does not start the quest.
            local dlg = click(bot, "진백랑", contains("대륙의 기운이"))
            dlg = dlg and next_dialog(bot, is_list)
            if dlg == nil or choose(bot, dlg, "아니오.. 싫습니다.") == false then
                return fail("진백랑 refusal flow broke")
            end
            if check(bot, "verify_quest", "refused", QUEST_LIGHTHOUSE, "none") == false then
                return fail("refusing started the quest")
            end

            -- Accepting starts the quest at step 1; PREV goes back one page.
            if accept_jinbaekrang(bot) == false then
                return fail("진백랑 accept flow broke")
            end
            if check(bot, "verify_quest", "accepted", QUEST_LIGHTHOUSE, "1") == false then
                return fail("accepting did not set step 1")
            end

            -- A quest stuck at step 0 is offered again and also moves to step 1.
            check(bot, "set_quest", "step0", QUEST_LIGHTHOUSE, "0")
            if accept_jinbaekrang(bot) == false then
                return fail("진백랑 step 0 flow broke")
            end
            if check(bot, "verify_quest", "step0_accepted", QUEST_LIGHTHOUSE, "1") == false then
                return fail("step 0 accept did not set step 1")
            end

            -- Without the diaries 진백랑 only reminds the player.
            if click(bot, "진백랑", contains("아직 일기 아홉장과")) == nil then
                return fail("진백랑 accepted no diaries")
            end
            press(bot, "NEXT")

            -- 진진 without 초보도시락 shows the default line.
            if click(bot, "진진", contains("도시락을 깜빡")) == nil then
                return fail("진진 offered the trade without 초보도시락")
            end
            press(bot, "NEXT")

            -- 진진 refusal keeps the lunch box and the sub-quest unstarted.
            bot:create_item("초보도시락", 1)
            dlg = click(bot, "진진", contains("그 도시락을 나에게"))
            if dlg == nil or choose(bot, dlg, "주기 싫은데요.") == false then
                return fail("진진 refusal flow broke")
            end
            if check(bot, "verify_quest", "jinjin_refused", QUEST_JINJIN, "none") == false
                or check(bot, "verify_items", "jinjin_refused_items", "초보도시락=1,선장의일기1=0") == false then
                return fail("진진 refusal changed the state")
            end

            -- A full inventory blocks the reward, and the player can retry after freeing space.
            bot:clear_inventory()
            bot:create_item("초보도시락", 2)
            for _ = 1, INVENTORY_CAPACITY - 1 do
                bot:create_item("목도", 1)
            end
            dlg = click(bot, "진진", contains("그 도시락을 나에게"))
            dlg = dlg and select_option(bot, dlg, "네, 그러지요.", contains("소지품이 가득 차서"))
            if dlg == nil then
                return fail("진진 full inventory flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "jinjin_full", QUEST_LIGHTHOUSE, "1") == false
                or check(bot, "verify_items", "jinjin_full_items", "초보도시락=2,선장의일기1=0") == false then
                return fail("진진 took the lunch box with a full inventory")
            end

            bot:clear_inventory()
            bot:create_item("초보도시락", 1)
            dlg = click(bot, "진진", contains("그 도시락을 나에게"))
            if dlg == nil then
                return fail("진진 does not offer the trade again after a full-inventory failure")
            end
            dlg = select_option(bot, dlg, "네, 그러지요.", contains("고맙소!"))
            if dlg == nil then
                return fail("진진 trade flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "jinjin_done", QUEST_JINJIN, "0:done") == false
                or check(bot, "verify_quest", "lighthouse_2", QUEST_LIGHTHOUSE, "2") == false
                or check(bot, "verify_items", "jinjin_done_items", "초보도시락=0,선장의일기1=1") == false then
                return fail("진진 trade did not give 선장의일기1 and step 2")
            end

            -- 퉁퉁대감: refuse, accept, short delivery, then 10 호박.
            dlg = click(bot, "퉁퉁대감", is_list)
            if dlg == nil or choose(bot, dlg, "아니오, 바빠서..") == false then
                return fail("퉁퉁대감 refusal flow broke")
            end
            if check(bot, "verify_quest", "tungtung_refused", QUEST_TUNGTUNG, "none") == false then
                return fail("퉁퉁대감 refusal started the sub-quest")
            end
            dlg = click(bot, "퉁퉁대감", is_list)
            dlg = dlg and select_option(bot, dlg, "네, 구해드리지요.", contains("호박 열 개 일세"))
            if dlg == nil then
                return fail("퉁퉁대감 accept flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "tungtung_started", QUEST_TUNGTUNG, "0") == false then
                return fail("퉁퉁대감 did not start the sub-quest")
            end
            bot:create_item("호박", 9)
            if click(bot, "퉁퉁대감", contains("아직 호박 열 개를")) == nil then
                return fail("퉁퉁대감 accepted 9 호박")
            end
            press(bot, "NEXT")
            -- create_item waits for the exact stack count, so rebuild the stack; the clear also takes 선장의일기1.
            bot:clear_inventory()
            bot:create_item("선장의일기1", 1)
            bot:create_item("호박", 10)
            dlg = click(bot, "퉁퉁대감", contains("잘 가져왔군"))
            dlg = dlg and next_dialog(bot, contains("약속대로 좋은걸"))
            if dlg == nil then
                return fail("퉁퉁대감 hand-over flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "tungtung_done", QUEST_TUNGTUNG, "0:done") == false
                or check(bot, "verify_quest", "lighthouse_3", QUEST_LIGHTHOUSE, "3") == false
                or check(bot, "verify_items", "tungtung_done_items", "호박=0,선장의일기2=1") == false then
                return fail("퉁퉁대감 did not give 선장의일기2 and step 3")
            end

            -- A finished link stays silent once the chain moved on.
            if click(bot, "퉁퉁대감", contains("....")) == nil then
                return fail("퉁퉁대감 answered after the chain moved on")
            end
            press(bot, "NEXT")

            -- 통통대감: accept, no 망치, then 망치.
            dlg = click(bot, "통통대감", contains("무기 수집가"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "네, 구해드리겠습니다.", contains("이 일기같은건"))
            if dlg == nil then
                return fail("통통대감 accept flow broke")
            end
            press(bot, "NEXT")
            if click(bot, "통통대감", contains("아직 망치라는")) == nil then
                return fail("통통대감 accepted no 망치")
            end
            press(bot, "NEXT")
            bot:create_item("망치", 1)
            dlg = click(bot, "통통대감", contains("묵직하고 단단한"))
            dlg = dlg and next_dialog(bot, contains("약속했던 누군가의 일기"))
            if dlg == nil then
                return fail("통통대감 hand-over flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "tongtong_done", QUEST_TONGTONG, "0:done") == false
                or check(bot, "verify_quest", "lighthouse_4", QUEST_LIGHTHOUSE, "4") == false
                or check(bot, "verify_items", "tongtong_done_items", "망치=0,선장의일기3=1") == false then
                return fail("통통대감 did not give 선장의일기3 and step 4")
            end

            -- 탕탕대감: accept, no 태존도, then 태존도.
            dlg = click(bot, "탕탕대감", contains("태존도에 대해"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "네, 구해다 드리겠습니다.", contains("나름대로 보답은"))
            if dlg == nil then
                return fail("탕탕대감 accept flow broke")
            end
            press(bot, "NEXT")
            if click(bot, "탕탕대감", contains("아직 태존도를")) == nil then
                return fail("탕탕대감 accepted no 태존도")
            end
            press(bot, "NEXT")
            bot:create_item("태존도", 1)
            dlg = click(bot, "탕탕대감", contains("그게 태존도인가"))
            dlg = dlg and next_dialog(bot, contains("이건 거래니까"))
            if dlg == nil then
                return fail("탕탕대감 hand-over flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "tangtang_done", QUEST_TANGTANG, "0:done") == false
                or check(bot, "verify_quest", "lighthouse_5", QUEST_LIGHTHOUSE, "5") == false
                or check(bot, "verify_items", "tangtang_done_items", "태존도=0,선장의일기4=1") == false then
                return fail("탕탕대감 did not give 선장의일기4 and step 5")
            end

            -- 준준: accept the totem request.
            dlg = open_pursuit(bot, "준준", "문화재보호공무원", contains("내부의 적과"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "네, 구해 보겠습니다.", contains("토템들을 좀 찾아다"))
            if dlg == nil then
                return fail("준준 accept flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "junjun_started", QUEST_JUNJUN, "0") == false then
                return fail("준준 did not start the sub-quest")
            end

            -- No reward before 5 totems, and a missing totem is refused.
            if claim_totem_reward(bot, contains("적어도 5번은")) == false then
                return fail("준준 rewarded before any totem")
            end
            dlg = open_pursuit(bot, "준준", "문화재보호공무원", is_list)
            dlg = dlg and select_option(bot, dlg, "토템을 하나 찾아왔습니다", is_list)
            dlg = dlg and select_option(bot, dlg, "번개의토템", contains("없는데?"))
            if dlg == nil then
                return fail("준준 accepted a missing totem")
            end
            press(bot, "NEXT")

            local totems = { "번개의토템", "바람의토템", "대지의토템", "화염의토템" }
            -- Totems do not stack: 4+4+4+3 single items.
            for i = 1, 15 do
                bot:create_item(totems[((i - 1) % #totems) + 1], 1)
            end
            local delivered = 0
            local function deliver(count)
                for _ = 1, count do
                    local totem = totems[(delivered % #totems) + 1]
                    if deliver_totem(bot, totem) == false then
                        return false
                    end
                    delivered = delivered + 1
                end
                return true
            end

            -- 5 totems: 선장의일기5, sub-quest step 1. A second claim gives nothing.
            if deliver(5) == false then
                return fail("totem delivery flow broke")
            end
            if claim_totem_reward(bot, contains("수고했네. 계속 힘내주게.")) == false then
                return fail("준준 did not reward 5 totems")
            end
            if check(bot, "verify_quest", "junjun_1", QUEST_JUNJUN, "1") == false
                or check(bot, "verify_items", "junjun_1_items", "선장의일기5=1") == false then
                return fail("5 totems did not give 선장의일기5")
            end
            if claim_totem_reward(bot, contains("적어도 5번은")) == false
                or check(bot, "verify_items", "junjun_1_again", "선장의일기5=1,반룡곤=0") == false then
                return fail("준준 rewarded the same tier twice")
            end

            -- 10 totems: 반룡곤. 15 totems: 가시나무봉, completion and chain step 6.
            if deliver(5) == false or claim_totem_reward(bot, contains("수고했네. 계속 힘내주게.")) == false then
                return fail("10 totem flow broke")
            end
            if check(bot, "verify_quest", "junjun_2", QUEST_JUNJUN, "2") == false
                or check(bot, "verify_items", "junjun_2_items", "반룡곤=1") == false then
                return fail("10 totems did not give 반룡곤")
            end
            if deliver(5) == false or claim_totem_reward(bot, contains("수고했네. 계속 힘내주게.")) == false then
                return fail("15 totem flow broke")
            end
            if check(bot, "verify_quest", "junjun_done", QUEST_JUNJUN, "3:done") == false
                or check(bot, "verify_quest", "lighthouse_6", QUEST_LIGHTHOUSE, "6") == false
                or check(bot, "verify_items", "junjun_done_items",
                    "가시나무봉=1,번개의토템=0,바람의토템=0,대지의토템=0,화염의토템=0") == false then
                return fail("15 totems did not complete 준준 and set step 6")
            end
            dlg = open_pursuit(bot, "준준", "문화재보호공무원", contains("일은 정말 힘들군"))
            if dlg == nil then
                return fail("준준 still runs the quest after completion")
            end
            press(bot, "NEXT")

            -- 진백랑 needs diaries 1-9 and the cover; 6-9 and the cover come from elsewhere.
            if click(bot, "진백랑", contains("아직 일기 아홉장과")) == nil then
                return fail("진백랑 accepted diaries 1-5 only")
            end
            press(bot, "NEXT")
            for _, name in ipairs({ "선장의일기6", "선장의일기7", "선장의일기8", "선장의일기9", "선장의일기겉표지" }) do
                bot:create_item(name, 1)
            end
            dlg = click(bot, "진백랑", contains("일기장을 정리해오겠네"))
            dlg = dlg and next_dialog(bot, contains("여기 일기장이"))
            dlg = dlg and next_dialog(bot, contains("등대빛의 검이라"))
            if dlg == nil then
                return fail("진백랑 completion flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "lighthouse_done", QUEST_LIGHTHOUSE, "6:done") == false
                or check(bot, "verify_items", "lighthouse_done_items",
                    "선장의일기1=0,선장의일기5=0,선장의일기9=0,선장의일기겉표지=0,선장의일기장=1,등대빛의검=1") == false then
                return fail("진백랑 did not take the diaries and give the rewards")
            end
            if click(bot, "진백랑", contains("......")) == nil then
                return fail("진백랑 still runs the quest after completion")
            end
            press(bot, "NEXT")
            return true
        end,
    },
}
