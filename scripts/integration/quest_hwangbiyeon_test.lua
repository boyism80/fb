-- QUEST_HWANGBIYEON (195): 장안성장군(npc/231) -> 상해주민(npc/232) -> 도재영(npc/107)
-- -> 상해주민 -> 장안성장군.
-- 도재영 offers QUEST_DOJAEYOUNG_HERB (196) from QUEST_HWANGBIYEON step 2 on, or once 196 has
-- started; the herb quest and its hand-off to 도성연(npc/108) are covered on the way.
-- Expected values come from the NPC scripts.
local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_HWANGBIYEON = 195
local QUEST_HERB        = 196
local START_MONEY       = 5000
local BOUNTY            = 100000

local contains, is_list, check = qd.contains, qd.is_list, qd.check
local click, press, choose, next_dialog, select_option = qd.click, qd.press, qd.choose, qd.next_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "hwangbiyeon: " .. string.format(fmt, ...))
    return false
end

-- Runs a chain of (NEXT, expect) steps from the current dialog.
local function next_chain(bot, dlg, ...)
    for _, expect in ipairs({ ... }) do
        dlg = dlg and next_dialog(bot, contains(expect))
    end
    return dlg
end

local function open_dojaeyoung(bot, option, expect)
    local dlg = click(bot, "도재영", contains("무엇을 도와드릴까요"))
    return dlg, dlg and select_option(bot, dlg, option, expect)
end

test_suite {
    name      = "Quest Hwangbiyeon Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        check(bot, "set_quest", "reset_main", QUEST_HWANGBIYEON, "none")
        check(bot, "set_quest", "reset_herb", QUEST_HERB, "none")
        bot:clear_inventory()
        qd.remove_npc(bot)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "init_main", QUEST_HWANGBIYEON, "none") == false
                or check(bot, "set_quest", "init_herb", QUEST_HERB, "none") == false
                or check(bot, "set_money", "init_money", START_MONEY) == false then
                return fail("could not reset the quests")
            end
            bot:clear_inventory()

            if click(bot, "상해주민", contains("안녕하신가요?")) == nil then
                return fail("상해주민 talked about 황비연 before the quest")
            end
            press(bot, "NEXT")

            -- 장안성장군: the short answer ends the talk without a quest.
            local dlg = click(bot, "장안성장군", contains("황궁을 견학하러"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "전 아무것도 모릅니다. 그럼 이만...", contains("싱겁기는"))
            if dlg == nil then
                return fail("장안성장군 refusal flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "general_refused", QUEST_HWANGBIYEON, "none") == false then
                return fail("refusing started the quest")
            end

            dlg = click(bot, "장안성장군", contains("황궁을 견학하러"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "잘 모르겠는데요. 그게 누굽니까?", contains("신출귀몰한 도둑"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "의적이군요? 그럼 슬쩍 눈감아줘도 되지 않을까요?", contains("상부에서 그를"))
            dlg = dlg and next_dialog(bot, contains("도둑질은 나쁜 일"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "저도 협조하겠습니다. 황비연을 만나보고 싶군요.", contains("정말 그래 주겠는가"))
            dlg = dlg and next_dialog(bot, contains("마지막으로 나타난 장소는 상해"))
            if dlg == nil then
                return fail("장안성장군 accept flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "general_accepted", QUEST_HWANGBIYEON, "1") == false then
                return fail("장안성장군 did not set step 1")
            end

            if click(bot, "장안성장군", contains("한번 상해로 가보십시요")) == nil then
                return fail("장안성장군 step 1 hint missing")
            end
            press(bot, "NEXT")

            -- 도재영 has nothing to do before step 2 and hides the herb quest.
            local menu
            menu, dlg = open_dojaeyoung(bot, "황비연 퀘스트", is_list)
            if menu == nil or #(menu.list_lists or {}) ~= 1 then
                return fail("도재영 menu at step 1 is not just the 황비연 option")
            end
            dlg = dlg and select_option(bot, dlg, "상해에서 물건을 전해달라는 부탁을 받고 왔습니다.", contains("아직 제게 맡기실 일이"))
            if dlg == nil then
                return fail("도재영 step 1 flow broke")
            end
            press(bot, "NEXT")

            -- 상해주민: the bounty answer ends the talk; meeting him gives 노비문서 (step 2).
            dlg = click(bot, "상해주민", is_list)
            dlg = dlg and select_option(bot, dlg, "황비연이라는 사람을 찾고 있습니다.", contains("의적 아닙니까"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "그를 잡아서 현상금을 타려고 합니다.", contains("쉽게 잡기는 어려울"))
            if dlg == nil then
                return fail("상해주민 bounty flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "villager_bounty", QUEST_HWANGBIYEON, "1") == false
                or check(bot, "verify_items", "villager_bounty_items", "노비문서=0") == false then
                return fail("the bounty answer advanced the quest")
            end

            dlg = click(bot, "상해주민", is_list)
            dlg = dlg and select_option(bot, dlg, "황비연이라는 사람을 찾고 있습니다.", contains("의적 아닙니까"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "꼭 한 번 만나 보고 싶은 사람입니다.", contains("도움을 받으셨었던"))
            dlg = next_chain(bot, dlg, "......", "제 부탁 하나만", "황비연을 좀 알고")
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "알겠습니다, 그렇게 하죠.", contains("노비문서를 도삭산 100층"))
            dlg = next_chain(bot, dlg, "잘 부탁드립니다")
            if dlg == nil then
                return fail("상해주민 request flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "villager_request", QUEST_HWANGBIYEON, "2") == false
                or check(bot, "verify_items", "villager_request_items", "노비문서=1") == false then
                return fail("상해주민 did not give 노비문서 and step 2")
            end

            if click(bot, "상해주민", contains("아직 도삭산 100층에")) == nil then
                return fail("상해주민 step 2 hint missing")
            end
            press(bot, "NEXT")

            -- Step 2 opens the herb quest at 도재영.
            menu, dlg = open_dojaeyoung(bot, "약초 퀘스트", contains("도삭산을 탐험하고"))
            if menu == nil or #(menu.list_lists or {}) ~= 2 then
                return fail("도재영 menu at step 2 does not offer the herb quest")
            end
            dlg = next_chain(bot, dlg, "건강이 안좋으시죠")
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "제가 구해드리겠습니다.", contains("마음도 넓으신"))
            dlg = next_chain(bot, dlg, "151층에서 200층")
            if dlg == nil then
                return fail("도재영 herb start flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "herb_started", QUEST_HERB, "1") == false then
                return fail("도재영 did not start the herb quest at step 1")
            end

            _, dlg = open_dojaeyoung(bot, "약초 퀘스트", contains("아직 약초를"))
            if dlg == nil then
                return fail("도재영 accepted no herbs")
            end
            press(bot, "NEXT")

            bot:create_item("약초잎사귀", 5)
            bot:create_item("약초가지", 1)
            _, dlg = open_dojaeyoung(bot, "약초 퀘스트", contains("약초잎사귀와 약초가지를 구하셨군요"))
            dlg = next_chain(bot, dlg, "강철의구두를 드리겠습니다")
            if dlg == nil then
                return fail("도재영 herb turn-in flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "herb_done", QUEST_HERB, "2") == false
                or check(bot, "verify_items", "herb_done_items", "약초잎사귀=0,약초가지=0,강철의구두=1") == false then
                return fail("herb turn-in did not give 강철의구두 and step 2")
            end

            -- 도성연 continues the herb chain: declining keeps step 2, accepting moves to step 3.
            dlg = click(bot, "도성연", contains("아들놈에게 약초를"))
            dlg = next_chain(bot, dlg, "선물을 하나 주려고", "인어의방울이라고", "방울 하나쯤")
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "저는 좀 바빠서..", contains("바쁘시다면"))
            if dlg == nil then
                return fail("도성연 decline flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "father_declined", QUEST_HERB, "2") == false then
                return fail("declining 도성연 changed the herb quest")
            end
            dlg = click(bot, "도성연", contains("아들놈에게 약초를"))
            dlg = next_chain(bot, dlg, "선물을 하나 주려고", "인어의방울이라고", "방울 하나쯤")
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "제가 구해드리겠습니다.", contains("고양이목에 방울달기"))
            dlg = next_chain(bot, dlg, "지름길로 보내주지")
            dlg = dlg and next_dialog(bot, is_list)
            if dlg == nil or choose(bot, dlg, "나중에 가겠습니다.") == false then
                return fail("도성연 accept flow broke")
            end
            if check(bot, "verify_quest", "father_accepted", QUEST_HERB, "3") == false then
                return fail("도성연 did not set herb step 3")
            end

            -- 도재영 exchanges 노비문서 for 보패 (step 3).
            _, dlg = open_dojaeyoung(bot, "황비연 퀘스트", is_list)
            dlg = dlg and select_option(bot, dlg, "상해에서 물건을 전해달라는 부탁을 받고 왔습니다.", contains("황비연의 노비문서군요"))
            dlg = dlg and select_option(bot, dlg, "예, 실은...", contains("그게 사실입니까"))
            dlg = next_chain(bot, dlg, "참 대견스럽군요")
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, ".....", contains("이 보패를 그에게"))
            dlg = next_chain(bot, dlg, "아주 자랑스럽게")
            if dlg == nil then
                return fail("도재영 document flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "document_given", QUEST_HWANGBIYEON, "3") == false
                or check(bot, "verify_items", "document_given_items", "노비문서=0,보패=1") == false then
                return fail("도재영 did not exchange 노비문서 for 보패")
            end

            -- Past step 2 the herb menu stays, and a finished herb step only gets thanks.
            menu, dlg = open_dojaeyoung(bot, "약초 퀘스트", contains("저번에 약초를 구해주셔서"))
            if menu == nil or #(menu.list_lists or {}) ~= 2 or dlg == nil then
                return fail("도재영 hid the herb quest after step 2")
            end
            press(bot, "NEXT")

            -- 상해주민 at step 3 needs 보패.
            bot:clear_inventory()
            if click(bot, "상해주민", contains("아직 도삭산 100층에")) == nil then
                return fail("상해주민 continued without 보패")
            end
            press(bot, "NEXT")
            bot:create_item("보패", 1)
            dlg = click(bot, "상해주민", is_list)
            dlg = dlg and select_option(bot, dlg, "예, 확실히 전해 줬습니다.", contains("이 보패는 웬 것"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "예, 당신을 자랑스러워하셨습니다. 황비연씨.", contains("제가 황비연입니다"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "그럴 수는 없습니다.", is_list)
            dlg = dlg and select_option(bot, dlg, "당신은 의적으로서 많은 사람을 도왔죠.", is_list)
            dlg = dlg and select_option(bot, dlg, "그런 사람을 체포할 수는 없습니다.", is_list)
            dlg = dlg and select_option(bot, dlg, "앞으로 좋은 일을 더 많이 하시면 되지 않겠습니까?", contains("정말로 그럴까요"))
            dlg = next_chain(bot, dlg, "제가 쓰던 머리띠입니다", "드릴 것이 없군요", "떳떳한 방법으로")
            if dlg == nil then
                return fail("황비연 reveal flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "headband_given", QUEST_HWANGBIYEON, "4") == false
                or check(bot, "verify_items", "headband_given_items", "보패=0,황비연의머리띠=1") == false then
                return fail("황비연 did not exchange 보패 for the headband")
            end

            -- 장안성장군 at step 4: no headband, then the bounty.
            bot:clear_inventory()
            dlg = click(bot, "장안성장군", is_list)
            dlg = dlg and select_option(bot, dlg, "그는 죽었고, 그의 머리띠를 가져왔습니다.", contains("머리띠가 없는것"))
            if dlg == nil then
                return fail("장안성장군 accepted no headband")
            end
            press(bot, "NEXT")
            bot:create_item("황비연의머리띠", 1)
            dlg = click(bot, "장안성장군", is_list)
            dlg = dlg and select_option(bot, dlg, "그는 죽었고, 그의 머리띠를 가져왔습니다.", contains("그 머리띠가 분명하군"))
            dlg = next_chain(bot, dlg, "현상금 10만전")
            if dlg == nil then
                return fail("장안성장군 bounty flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "bounty_paid", QUEST_HWANGBIYEON, "4:done") == false
                or check(bot, "verify_items", "bounty_paid_items",
                    string.format("황비연의머리띠=0,money=%d", START_MONEY + BOUNTY)) == false then
                return fail("장안성장군 did not pay the bounty and complete the quest")
            end

            if click(bot, "장안성장군", contains("진급을하여")) == nil then
                return fail("장안성장군 still runs the quest after completion")
            end
            press(bot, "NEXT")
            return true
        end,
    },
}
