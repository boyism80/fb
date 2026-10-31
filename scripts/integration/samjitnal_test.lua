local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_SAMJIT_HWAMYEON = 273
local SAMJITNAL_CLOCK       = "2026-04-19 12:00:00"
local CLOCK_WAIT_MS         = 1500

local contains, is_list, check = qd.contains, qd.is_list, qd.check
local click, press, next_dialog, select_option = qd.click, qd.press, qd.next_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "samjitnal: " .. string.format(fmt, ...))
    return false
end

local function open_butterfly_menu(bot)
    local dlg = click(bot, "장명주", contains("저는 장명주입니다"))
    dlg = dlg and next_dialog(bot, contains("화창한 봄이죠"))
    dlg = dlg and next_dialog(bot, is_list)
    return dlg
end

local function ask_for_help(bot)
    local dlg = open_butterfly_menu(bot)
    dlg = dlg and select_option(bot, dlg, "나비를 잡아왔어요.", contains("집안에만"))
    dlg = dlg and next_dialog(bot, contains("놀러가지도 못하고"))
    dlg = dlg and next_dialog(bot, contains("손님이 계신데"))
    dlg = dlg and next_dialog(bot, contains("나비를 좀 잡아주시겠어요"))
    dlg = dlg and next_dialog(bot, is_list)
    return dlg
end

test_suite {
    name      = "Samjitnal Test",
    bot_count = 1,

    on_initialize = function(ctx)
        local bot = ctx:bot(0)
        lib.formation.arrange_in_line(ctx)
        bot:chat("/현재시간 " .. SAMJITNAL_CLOCK)
        ctx:sleep(CLOCK_WAIT_MS)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        bot:clear_inventory()
    end,

    on_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.remove_npc(bot)
        check(bot, "set_quest", "samjit_reset_end", QUEST_SAMJIT_HWAMYEON, "none")
        bot:chat("/현재시간초기화")
        ctx:sleep(CLOCK_WAIT_MS)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot:clear_inventory()
            if check(bot, "verify_clock_hour", "samjit_clock", "12") == false then
                return fail("could not set the clock to samjitnal")
            end
            if check(bot, "set_quest", "samjit_reset", QUEST_SAMJIT_HWAMYEON, "none") == false then
                return fail("could not reset the hwamyeon quest")
            end

            local dlg = ask_for_help(bot)
            dlg = dlg and select_option(bot, dlg, "나비 잡을시간 없는데..", contains("어쩔 수 없죠"))
            dlg = dlg and next_dialog(bot, contains("만들어 둔거라도"))
            if dlg == nil then
                return fail("the first refusal did not offer hwamyeon")
            end
            press(bot, "NEXT")
            ctx:sleep(500)
            if check(bot, "verify_items", "samjit_first", "맛없는화면=1") == false then
                return fail("the first refusal did not give one hwamyeon")
            end
            if check(bot, "verify_quest", "samjit_done", QUEST_SAMJIT_HWAMYEON, "0:done") == false then
                return fail("the hwamyeon quest was not completed")
            end

            dlg = ask_for_help(bot)
            dlg = dlg and select_option(bot, dlg, "나비 잡을시간 없는데..", contains("어쩔 수 없죠"))
            if dlg == nil then
                return fail("the second refusal did not answer")
            end
            press(bot, "NEXT")
            ctx:sleep(500)
            if check(bot, "verify_items", "samjit_second", "맛없는화면=1") == false then
                return fail("the second refusal in the same year gave hwamyeon again")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            bot:clear_inventory()
            if check(bot, "make_butterfly_jar", "samjit_empty_jar", 0) == false then
                return fail("could not create an empty jar")
            end

            local dlg = open_butterfly_menu(bot)
            dlg = dlg and select_option(bot, dlg, "나비를 잡아왔어요.", contains("집안에만"))
            if dlg == nil then
                return fail("an empty jar was taken as caught butterflies")
            end
            qd.close(bot)
            ctx:sleep(500)
            if check(bot, "verify_butterfly_jar", "samjit_empty_kept", 0, 1) == false then
                return fail("an empty jar was taken by myeongju")
            end
            if check(bot, "verify_items", "samjit_empty_no_reward", "보통화면=0,맛없는화면=0") == false then
                return fail("an empty jar was exchanged for hwamyeon")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            bot:clear_inventory()
            if check(bot, "make_items", "samjit_net", "나비채", 1, 0) == false then
                return fail("could not create a butterfly net")
            end

            local dlg = click(bot, "오선릉", contains("저는 오선릉입니다"))
            dlg = dlg and next_dialog(bot, is_list)
            dlg = dlg and select_option(bot, dlg, "나비채를 좀 주세요!", contains("찾아볼께요"))
            dlg = dlg and next_dialog(bot, contains("10마리"))
            dlg = dlg and next_dialog(bot, contains("흔들면"))
            if dlg == nil then
                return fail("oseonreung did not hand out the jar")
            end
            press(bot, "NEXT")
            ctx:sleep(500)
            if check(bot, "verify_items", "samjit_one_net", "나비채=1") == false then
                return fail("oseonreung gave another butterfly net")
            end
            if check(bot, "verify_butterfly_jar", "samjit_jar_given", 0, 1) == false then
                return fail("oseonreung did not give the empty jar")
            end
            return true
        end,
    },
}
