local lib         = require("integration.lib")
local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local trade       = require("integration.lib.trade")
local script_step = require("integration.lib.script_step")
local qd          = require("integration.lib.quest_dialog")

local F1_OID = 0xFFFFFFFF
local F1_STORAGE_OPTION = "통합보관함"

local MESSAGE_TRADE_BLOCKED_WHILE_TRADING = "교환 중에는 할 수 없습니다."
local MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE  = "교환이 불가능한 아이템입니다."
local MESSAGE_TRADE_CANCELLED_BY_ME       = "내가 교환을 취소했습니다."
local MESSAGE_TRADE_SUCCESS               = "교환에 성공했습니다."
local MESSAGE_TRADE_FAILED                = "교환에 실패했습니다."
local MESSAGE_ITEM_FULL                   = "더 이상 가질 수 없습니다."
local MESSAGE_TRADE_PARTNER_BUSY          = "다른 일을 하는 중입니다."

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
        for i = 0, ctx:bot_count() - 1 do
            qd.close(ctx:bot(i))
        end
        progress(ctx:bot(0), "FORMATION DONE")
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            progress(bot, "CLEANUP INVENTORY")
            qd.remove_npc(bot)
            bot:clear_inventory()
            bot:chat("/아이템삭제")
            bot:money(0)
        end
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

            lib.wait.state(bot1, resp.update_internal, function()
                return bot1:money() == 20000
            end)

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
            local ENCODED_PAST_U32 = 43

            progress(bot1, "SCENARIO 2 START (money past uint32 / display encode)")

            bot1:money(0xFFFFFFFF)
            bot2:money(1)
            progress(bot1, "SET MONEY FORMER MAX / bot2=1")

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot2, "UP MONEY 1")
            trade.up_money(bot2, bot1:oid(), 1)

            progress(bot1, "LOCK TRADE")
            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))

            progress(bot2, "LOCK TRADE (COMPLETE)")
            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_SUCCESS))

            lib.wait.state(bot1, resp.update_internal, function()
                return bot1:money() == ENCODED_PAST_U32
            end)

            if bot1:money() ~= ENCODED_PAST_U32 or bot2:money() ~= 0 then
                progress(bot1, "FAILED: money past-uint32 encode outcome wrong")
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

            -- bot2 cannot hold the incoming stack, so bot2 gets the cause instead of the generic failure.
            progress(bot2, "LOCK TRADE (expect fail)")
            if trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_ITEM_FULL)) == nil then
                progress(bot2, "FAILED: cause message missing for the side that cannot hold the stack")
                return false
            end
            progress(bot2, "TRADE FAILED AS EXPECTED")

            lib.wait.state(bot1, resp.item_update, function()
                return bot1:item_count("도토리") == 150
            end)

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

            -- bot1's inventory is full; bot2 is not the cause, so the side locking last sees the generic failure.
            progress(bot2, "LOCK TRADE (expect generic fail)")
            if trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_FAILED)) == nil then
                progress(bot2, "FAILED: generic failure message missing for the side that is not the cause")
                return false
            end

            progress(bot1, "REQUEST TRADE -> bot2 (cause locks last)")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))
            trade.up_item_upload(bot2, bot1:oid(), 0)
            trade.lock(bot2, bot1:oid(), trade.type_is("lock"))

            progress(bot1, "LOCK TRADE (expect cause)")
            if trade.lock(bot1, bot2:oid(), trade.close_contains(MESSAGE_ITEM_FULL)) == nil then
                progress(bot1, "FAILED: full-inventory side did not get the cause message")
                return false
            end
            progress(bot2, "TRADE FAILED AS EXPECTED")

            lib.wait.state(bot2, resp.item_update, function()
                return bot2:has_item_by_name("현철중검")
            end)
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
            if trade.lock(bot1, bot2:oid(), trade.close_contains(MESSAGE_ITEM_FULL)) == nil then
                progress(bot1, "FAILED: cause message missing for the side that cannot hold the stack")
                return false
            end
            progress(bot1, "TRADE FAILED AS EXPECTED")

            lib.wait.state(bot2, resp.item_update, function()
                return bot2:item_count("도토리") == 200
            end)

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

            lib.wait.state(bot2, resp.item_update, function()
                return bot2:item_count("도토리") == 99
            end)

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

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 6 START (an offered stack cannot be dropped by script)")

            bot1:create_item("도토리", 50)

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot1, "UP 도토리 x50")
            trade.up_item(bot1, bot2:oid(), 0)
            trade.item_count(bot1, bot2:oid(), 50)

            progress(bot1, "DROP OFFERED STACK BY SCRIPT (expect reject)")
            if script_step.run_script(bot1, "test/integration", "drop_slot", "trade_drop", "1") == true then
                progress(bot1, "FAILED: a script dropped the offered stack while trading")
                return false
            end

            progress(bot1, "CANCEL (the trade must still be open)")
            trade.cancel(bot1, bot2:oid(), trade.close_contains(MESSAGE_TRADE_CANCELLED_BY_ME))
            lib.wait.state(bot1, resp.item_update, function()
                return bot1:item_count("도토리") == 50
            end)
            if bot1:item_count("도토리") ~= 50 or bot2:item_count("도토리") ~= 0 then
                progress(bot1, string.format("FAILED: after cancel bot1=%d bot2=%d 도토리", bot1:item_count("도토리"),
                    bot2:item_count("도토리")))
                return false
            end

            progress(bot1, "SCENARIO 6 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 7 START (part of an offered stack leaves during the trade)")

            bot1:create_item("도토리", 10)
            bot2:money(1000)

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot1, "UP 도토리 x10")
            trade.up_item(bot1, bot2:oid(), 0)
            trade.item_count(bot1, bot2:oid(), 10)

            progress(bot2, "UP MONEY 1000 AND LOCK")
            trade.up_money(bot2, bot1:oid(), 1000)
            trade.lock(bot2, bot1:oid(), trade.type_is("lock"))

            -- An NPC sale or a quest can take part of the stack while the trade window is open.
            progress(bot1, "REMOVE 도토리 x5 BY SCRIPT")
            if script_step.run_script(bot1, "test/integration", "remove_items", "trade_take_part", "도토리", "5") ~= true then
                progress(bot1, "FAILED: script removal of part of the offered stack")
                return false
            end

            progress(bot1, "LOCK (the trade must already be cancelled)")
            bot1:send(protocol.trade("LOCK", bot2:oid(), {}))
            ctx:sleep(1000)

            if bot2:item_count("도토리") ~= 0 then
                progress(bot2, string.format("FAILED: bot2 received %d 도토리 from a trade that lost part of its offer",
                    bot2:item_count("도토리")))
                return false
            end
            if bot2:money() ~= 1000 or bot1:money() ~= 0 then
                progress(bot1, string.format("FAILED: money moved (bot1=%d bot2=%d)", bot1:money(), bot2:money()))
                return false
            end
            if bot1:item_count("도토리") ~= 5 then
                progress(bot1, string.format("FAILED: bot1 도토리 shows %d, expected 5", bot1:item_count("도토리")))
                return false
            end

            progress(bot1, "SCENARIO 7 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 8 START (NPC work is blocked while trading)")

            bot1:create_item("진호박", 5)
            qd.use_npc(bot1, "왈숙네")

            progress(bot1, "REQUEST TRADE -> bot2")
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            progress(bot1, "F1 MENU WHILE TRADING (expect reject)")
            local blocked = bot1:request(resp.message, protocol.click(F1_OID), function(packet)
                return string.find(packet.text or "", MESSAGE_TRADE_BLOCKED_WHILE_TRADING, 1, true) ~= nil
            end)
            if type(blocked) ~= "table" then
                progress(bot1, "FAILED: F1 menu opened while trading")
                return false
            end

            progress(bot1, "SELL TO NPC BY CHAT WHILE TRADING (expect ignored)")
            bot1:send(protocol.chat(false, "진호박 5개 판다"))
            ctx:sleep(1000)
            if bot1:item_count("진호박") ~= 5 or bot1:money() ~= 0 then
                progress(bot1, string.format("FAILED: NPC chat sale ran while trading (진호박=%d money=%d)",
                    bot1:item_count("진호박"), bot1:money()))
                return false
            end

            trade.cancel(bot1, bot2:oid(), trade.close_contains(MESSAGE_TRADE_CANCELLED_BY_ME))

            progress(bot1, "SELL TO NPC BY CHAT AFTER THE TRADE")
            local sold = bot1:request(resp.update_internal, protocol.chat(false, "진호박 5개 판다"), function(packet)
                return packet.ch_money > 0
            end)
            if type(sold) ~= "table" then
                progress(bot1, "FAILED: the NPC chat sale does not work outside a trade either")
                return false
            end
            qd.remove_npc(bot1)

            progress(bot1, "OPEN F1 MENU, THEN bot1 REQUESTS A TRADE (bot1's menu must close)")
            if bot1:request_dialog(protocol.click(F1_OID), function(p) return p.type == "pursuit" end) == nil then
                progress(bot1, "FAILED: F1 menu did not open")
                return false
            end
            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))

            local seq = bot1:last_dialog()
            bot1:send(protocol.dialog("PURSUIT", 0, "", 0, 0, F1_STORAGE_OPTION))
            ctx:sleep(1000)
            if bot1:last_dialog() ~= seq then
                progress(bot1, "FAILED: the F1 menu kept running after the trade started")
                return false
            end

            trade.cancel(bot1, bot2:oid(), trade.close_contains(MESSAGE_TRADE_CANCELLED_BY_ME))

            progress(bot2, "OPEN F1 MENU, THEN bot1 REQUESTS A TRADE (refused, bot2's menu keeps running)")
            if bot2:request_dialog(protocol.click(F1_OID), function(p) return p.type == "pursuit" end) == nil then
                progress(bot2, "FAILED: F1 menu did not open")
                return false
            end
            local busy = bot1:request(resp.message, protocol.trade("REQUEST", bot2:oid(), {}), function(packet)
                return string.find(packet.text or "", MESSAGE_TRADE_PARTNER_BUSY, 1, true) ~= nil
            end)
            if type(busy) ~= "table" then
                progress(bot1, "FAILED: a trade with a partner in a dialog was not refused")
                return false
            end

            seq = bot2:last_dialog()
            bot2:send(protocol.dialog("PURSUIT", 0, "", 0, 0, F1_STORAGE_OPTION))
            ctx:sleep(1000)
            if bot2:last_dialog() == seq then
                progress(bot2, "FAILED: the F1 menu stopped after the refused trade")
                return false
            end
            qd.close(bot2)

            progress(bot1, "SCENARIO 8 PASSED")
            return true
        end,
    },
}
