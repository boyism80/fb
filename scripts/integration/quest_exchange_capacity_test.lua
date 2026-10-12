-- An exchange reward larger than the item's capacity is granted in full, one slot per capacity.
-- 배전(npc/32) sells 창힐독본 (capacity 1) x2 for 20000전 at QUEST_SKULL_NECKLACE (254) step 10.
local lib = require("integration.lib")
local qd  = require("integration.lib.quest_dialog")

local QUEST_SKULL_NECKLACE = 254
local PRICE                = 20000

local contains, is_list, check = qd.contains, qd.is_list, qd.check
local click, press, next_dialog, select_option = qd.click, qd.press, qd.next_dialog, qd.select_option

local function fail(fmt, ...)
    log("fatal", "exchange_capacity: " .. string.format(fmt, ...))
    return false
end

local function offer_books(bot)
    local dlg = click(bot, "배전", contains("창힐독본말인가"))
    dlg = dlg and next_dialog(bot, is_list)
    dlg = dlg and select_option(bot, dlg, "어떻게든 수를 내주세요.", contains("빌려둔 책이 두어권"))
    dlg = dlg and next_dialog(bot, contains("권당 1만전의 벌금"))
    return dlg and next_dialog(bot, is_list)
end

test_suite {
    name      = "Quest Exchange Capacity Test",
    bot_count = 1,

    on_initialize = function(ctx)
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        qd.close(bot)
        check(bot, "set_quest", "reset", QUEST_SKULL_NECKLACE, "none")
        bot:clear_inventory()
        qd.remove_npc(bot)
    end,

    scenarios = {
        function(ctx)
            local bot = ctx:bot(0)
            if check(bot, "set_quest", "init", QUEST_SKULL_NECKLACE, "10") == false
                or check(bot, "set_money", "init_money", PRICE) == false then
                return fail("could not set up QUEST_SKULL_NECKLACE step 10")
            end
            bot:clear_inventory()

            local dlg = offer_books(bot)
            dlg = dlg and select_option(bot, dlg, "음, 대신 물어드릴께요.", contains("수고했네"))
            if dlg == nil then
                return fail("배전 exchange flow broke")
            end
            press(bot, "NEXT")
            if check(bot, "verify_quest", "exchanged", QUEST_SKULL_NECKLACE, "11") == false
                or check(bot, "verify_items", "exchanged_items", "창힐독본=2,money=0") == false then
                return fail("배전 did not grant both 창힐독본 for 20000전")
            end
            return true
        end,
    },
}
