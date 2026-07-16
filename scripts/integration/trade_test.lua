local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")
local trade    = require("integration.lib.trade")

local MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE = "교환이 불가능한 아이템입니다."
local MESSAGE_TRADE_CANCELLED_BY_ME      = "내가 교환을 취소했습니다."
local MESSAGE_TRADE_SUCCESS              = "교환에 성공했습니다."
local MESSAGE_TRADE_FAILED               = "교환에 실패했습니다."

local DEFAULT_INTERVAL = 100

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil
        and message:find("AS EXPECTED", 1, true) == nil
        and message:find("SUCCESS:", 1, true) == nil then
        level = "fatal"
    end
    log(level, message)
    bot:chat("=== " .. message .. " ===")
end

local function setup_item(bot, label, item_name, count)
    progress(bot, string.format("%s: create %s x%d", label, item_name, count))
    bot:create_item(item_name, count)
end

local function setup_money(bot, label, amount)
    progress(bot, string.format("%s: set money %d", label, amount))
    bot:money(amount)
end

test_suite {
    name      = "Trade Test",
    bot_count = 2,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "TRADE TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
        progress(ctx:bot(0), "FORMATION DONE")
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            progress(bot, "CLEANUP INVENTORY")
            bot:chat("/아이템초기화")
            bot:chat("/아이템삭제")
            bot:money(0)
        end
        ctx:sleep(1000)
    end,

    scenarios = {
        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 1 START")

            setup_item(bot1, "bot1", "도토리", 150)
            setup_item(bot1, "bot1", "부적", 1)
            setup_money(bot1, "bot1", 10000)

            setup_item(bot2, "bot2", "도토리", 200)
            setup_item(bot2, "bot2", "양첨목봉", 1)
            setup_money(bot2, "bot2", 20000)

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot1, "TRY UPLOAD CHARM (expect reject)")
            local charm = bot1:request(
                resp.message,
                protocol.trade("UP_ITEM", bot2:oid(), { index = trade.slot(1) }),
                function(packet)
                    return string.find(packet.text or "", MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE, 1, true) ~= nil
                end)
            if charm == nil then
                progress(bot1, "FAILED: charm reject message missing")
                return false
            end
            progress(bot1, "CHARM REJECTED AS EXPECTED")

            progress(bot1, "PUT ITEMS AND MONEY")
            trade.put_full_offer(bot1, bot2:oid(), 150, 10000)

            progress(bot2, "PUT ITEMS AND MONEY")
            trade.put_full_offer(bot2, bot1:oid(), 200, 20000, 1)

            progress(bot2, "CANCEL TRADE")
            trade.cancel(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_CANCELLED_BY_ME))

            progress(bot2, "REQUEST TRADE -> bot1")
            trade.request(bot2, bot1:oid(), trade.dialog_oid(bot1:oid()))

            progress(bot1, "PUT ITEMS AND MONEY (2nd)")
            trade.put_full_offer(bot1, bot2:oid(), 150, 10000)

            progress(bot2, "PUT ITEMS AND MONEY (2nd)")
            trade.put_full_offer(bot2, bot1:oid(), 200, 20000, 1)

            progress(bot1, "LOCK TRADE")
            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))

            progress(bot2, "LOCK TRADE (COMPLETE)")
            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_SUCCESS))

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:money() ~= 20000 or bot2:money() ~= 10000 then
                progress(bot1, "FAILED: money mismatch")
                return false
            end
            if bot1:has_item_by_name("양첨목봉") == false then
                progress(bot1, "FAILED: missing 양첨목봉")
                return false
            end
            if bot1:has_item_by_name("부적") == false then
                progress(bot1, "FAILED: missing 부적")
                return false
            end
            if bot1:item_count("도토리") ~= 200 then
                progress(bot1, "FAILED: bot1 도토리 count")
                return false
            end
            if bot2:item_count("도토리") ~= 150 then
                progress(bot2, "FAILED: bot2 도토리 count")
                return false
            end

            progress(bot1, "SCENARIO 1 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 2 START (money overflow)")

            bot1:money(0xFFFFFFFF)
            bot2:money(1)
            progress(bot1, "SET MONEY MAX / bot2=1")

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot2, "UP MONEY 1")
            trade.up_money(bot2, bot1:oid(), 1)

            progress(bot1, "LOCK TRADE")
            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))

            progress(bot2, "LOCK TRADE (expect fail)")
            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            progress(bot2, "TRADE FAILED AS EXPECTED")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:money() ~= 0xFFFFFFFF or bot2:money() ~= 1 then
                progress(bot1, "FAILED: money changed after overflow reject")
                return false
            end

            progress(bot1, "SCENARIO 2 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 3 START (item stack overflow)")

            progress(bot1, "CREATE 도토리 x150")
            bot1:create_item("도토리", 150)
            progress(bot2, "CREATE 도토리 x150")
            bot2:create_item("도토리", 150)

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot1, "UP 도토리 x150")
            trade.up_item(bot1, bot2:oid(), 0)
            trade.item_count(bot1, bot2:oid(), 150)

            progress(bot1, "LOCK TRADE")
            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))

            progress(bot2, "LOCK TRADE (expect fail)")
            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            progress(bot2, "TRADE FAILED AS EXPECTED")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:item_count("도토리") ~= 150 or bot2:item_count("도토리") ~= 150 then
                progress(bot1, "FAILED: item count changed after stack overflow reject")
                return false
            end

            progress(bot1, "SCENARIO 3 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 4 START (inventory full)")

            progress(bot1, "FILL INVENTORY 목도")
            bot1:fill_inventory("목도")
            progress(bot2, "CREATE 현철중검")
            bot2:create_item("현철중검", 1)

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot2, "UP 현철중검")
            trade.up_item_upload(bot2, bot1:oid(), 0)

            progress(bot1, "LOCK TRADE")
            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))

            progress(bot2, "LOCK TRADE (expect fail)")
            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            progress(bot2, "TRADE FAILED AS EXPECTED")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:has_item_by_name("현철중검") or bot2:has_item_by_name("현철중검") == false then
                progress(bot1, "FAILED: inventory-full trade outcome wrong")
                return false
            end

            progress(bot1, "SCENARIO 4 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 5 START (partial stack then success)")

            progress(bot1, "FILL INVENTORY, DROP ONE, CREATE 도토리 x100")
            bot1:fill_inventory("목도")
            bot1:drop_item(0, false)
            bot1:create_item("도토리", 100)

            progress(bot2, "CREATE 도토리 x200")
            bot2:create_item("도토리", 200)

            progress(bot2, "REQUEST TRADE -> bot1")
            trade.request(bot2, bot1:oid(), trade.dialog_oid(bot1:oid()))

            progress(bot2, "UP 도토리 x200")
            trade.up_item(bot2, bot1:oid(), 0)
            trade.item_count(bot2, bot1:oid(), 200)

            progress(bot2, "LOCK TRADE")
            trade.lock(bot2, bot1:oid(), trade.type_is("lock"))

            progress(bot1, "LOCK TRADE (expect fail)")
            trade.lock(bot1, bot2:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            progress(bot1, "TRADE FAILED AS EXPECTED")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:item_count("도토리") ~= 100 or bot2:item_count("도토리") ~= 200 then
                progress(bot1, "FAILED: counts changed after first overflow reject")
                return false
            end

            progress(bot2, "REQUEST TRADE -> bot1 (retry)")
            trade.request(bot2, bot1:oid(), trade.dialog_oid(bot1:oid()))

            progress(bot2, "UP 도토리 x101")
            trade.up_item(bot2, bot1:oid(), 0)
            trade.item_count(bot2, bot1:oid(), 101)

            progress(bot2, "LOCK TRADE")
            trade.lock(bot2, bot1:oid(), trade.type_is("lock"))

            progress(bot1, "LOCK TRADE (COMPLETE)")
            trade.lock(bot1, bot2:oid(), trade.close_contains(MESSAGE_TRADE_SUCCESS))

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:item_count("도토리") ~= 201 then
                progress(bot1, "FAILED: bot1 도토리 != 201")
                return false
            end
            if bot2:item_count("도토리") ~= 99 then
                progress(bot2, "FAILED: bot2 도토리 != 99")
                return false
            end

            progress(bot1, "SCENARIO 5 PASSED")
            return true
        end,
    },
}
