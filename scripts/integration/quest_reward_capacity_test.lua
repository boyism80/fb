-- Quests whose first reward is an item: a full inventory must not start the quest,
-- so the player can accept again after freeing space instead of being charged for a re-issue.
-- QUEST_BONG_BOOK (181): 조염(npc/117). QUEST_NAMGUN (180): 나무꾼(npc/180).
local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_NAMGUN    = 180
local QUEST_BONG_BOOK = 181

local contains, is_list, check = qd.contains, qd.is_list, qd.check
local click, press, choose, next_dialog, select_option = qd.click, qd.press, qd.choose, qd.next_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "reward_capacity: " .. string.format(fmt, ...))
    return false
end

local function offer_axe(bot)
    local dlg = click(bot, "나무꾼", contains("나무를 하러 오셨소"))
    dlg = dlg and next_dialog(bot, contains("옷차람이 특이하시구려"))
    return dlg and next_dialog(bot, is_list)
end

test_suite {
    name      = "Quest Reward Capacity Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        check(bot, "set_quest", "reset_book", QUEST_BONG_BOOK, "none")
        check(bot, "set_quest", "reset_axe", QUEST_NAMGUN, "none")
        bot:clear_inventory()
        qd.remove_npc(bot)
    end,

    scenarios = {
        -- 조염 gives 봉래산전설 for free once; a later copy costs 1000전.
        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "init_book", QUEST_BONG_BOOK, "none") == false then
                return fail("could not reset QUEST_BONG_BOOK")
            end

            bot:fill_inventory("목도")
            local dlg = click(bot, "조염", contains("책 한권 드릴테니"))
            dlg = dlg and select_option(bot, dlg, "예, 주세요.", contains("소지품이 가득 차서"))
            if dlg == nil then
                return fail("조염 full inventory flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "book_full", QUEST_BONG_BOOK, "none") == false then
                return fail("조염 started the quest without giving the book")
            end

            bot:clear_inventory()
            dlg = click(bot, "조염", contains("책 한권 드릴테니"))
            if dlg == nil then
                return fail("조염 does not offer the free book again after a full-inventory failure")
            end
            dlg = select_option(bot, dlg, "예, 주세요.", contains("재밌게 읽게나"))
            if dlg == nil then
                return fail("조염 free book flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "book_given", QUEST_BONG_BOOK, "0") == false
                or check(bot, "verify_items", "book_given_items", "봉래산전설=1") == false then
                return fail("조염 did not give the book and start the quest")
            end

            if click(bot, "조염", contains("봉래산전설은 재미있나")) == nil then
                return fail("조염 does not recognise the held book")
            end
            press(bot, "NEXT")
            return true
        end,

        -- 나무꾼 lends 쇠도끼 for free once (level 30+); a later one costs 1만전.
        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "init_axe", QUEST_NAMGUN, "none") == false then
                return fail("could not reset QUEST_NAMGUN")
            end

            bot:level(29)
            if click(bot, "나무꾼", contains("많이 부족해 보이는군")) == nil then
                return fail("나무꾼 talked to a level 29 player")
            end
            press(bot, "NEXT")

            bot:level(30)
            bot:fill_inventory("목도")
            local dlg = offer_axe(bot)
            dlg = dlg and select_option(bot, dlg, "예, 빌려주세요.", contains("소지품이 가득 차서"))
            if dlg == nil then
                return fail("나무꾼 full inventory flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "axe_full", QUEST_NAMGUN, "none") == false then
                return fail("나무꾼 started the quest without giving the axe")
            end

            bot:clear_inventory()
            dlg = offer_axe(bot)
            if dlg == nil then
                return fail("나무꾼 does not offer the free axe again after a full-inventory failure")
            end
            dlg = select_option(bot, dlg, "예, 빌려주세요.", contains("도끼가 있으니 잘 쓰시오"))
            if dlg == nil then
                return fail("나무꾼 free axe flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "axe_given", QUEST_NAMGUN, "0") == false
                or check(bot, "verify_items", "axe_given_items", "쇠도끼=1") == false then
                return fail("나무꾼 did not give the axe and start the quest")
            end

            -- The second axe is paid; declining keeps everything unchanged.
            dlg = click(bot, "나무꾼", contains("다시 빌리려면 1만전"))
            if dlg == nil or choose(bot, dlg, "필요없어요.") == false then
                return fail("나무꾼 re-issue offer missing")
            end
            if check(bot, "verify_items", "axe_declined_items", "쇠도끼=1") == false then
                return fail("declining the re-issue changed the inventory")
            end
            return true
        end,
    },
}
