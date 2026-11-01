local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_DRAGON_KING = 210

local contains, is_list, check = qd.contains, qd.is_list, qd.check
local click, choose, next_dialog, select_option = qd.click, qd.choose, qd.next_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "quest_shark_bribe: " .. string.format(fmt, ...))
    return false
end

test_suite {
    name      = "Quest Shark Bribe Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        check(bot, "set_quest", "reset_dragon_king", QUEST_DRAGON_KING, "none")
        bot:clear_inventory()
        qd.remove_npc(bot)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            bot:clear_inventory()

            if check(bot, "verify_shark_bribe_once", "shark_bribe_once") == false then
                return fail("a shark general that was already bribed offered the bribe again")
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "report_init", QUEST_DRAGON_KING, "12") == false
                or check(bot, "set_quest_progress", "report_caught", QUEST_DRAGON_KING, "1") == false
                or check(bot, "set_quest_param", "report_param", QUEST_DRAGON_KING, "0") == false then
                return fail("could not prepare step 12 with the shark captain caught")
            end

            local dlg = click(bot, "용왕", contains("그래 상어장군을 잡아왔는가?"))
            dlg = dlg and select_option(bot, dlg, "예. 여기있습니다.", contains("저자는 상어장군이 아니지"))
            dlg = dlg and select_option(bot, dlg, "예? 분명히 저자가", contains("상어장수라네"))
            dlg = dlg and next_dialog(bot, is_list)
            if dlg == nil or choose(bot, dlg, "이런. 제가 다시 가서 잡아오도록 하겠습니다.") == false then
                return fail("용왕 shark report flow broke")
            end

            if check(bot, "verify_quest", "report_step", QUEST_DRAGON_KING, "13") == false
                or check(bot, "verify_quest_progress", "report_progress", QUEST_DRAGON_KING, "0") == false then
                return fail("the report did not reset the catch count for step 13")
            end

            qd.close(bot)
            if click(bot, "용왕", contains("다시 상어장군을 잡아오게나")) == nil then
                return fail("용왕 gave the step 13 reward without the real shark general")
            end
            if check(bot, "verify_quest", "report_no_reward", QUEST_DRAGON_KING, "13") == false then
                return fail("the step 13 reward was given right after the report")
            end
            return true
        end,
    },
}
