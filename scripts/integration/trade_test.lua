local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")
local trade    = require("integration.lib.trade")

local MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE = "교환이 불가능한 아이템입니다."
local MESSAGE_TRADE_CANCELLED_BY_ME      = "내가 교환을 취소했습니다."
local MESSAGE_TRADE_SUCCESS              = "교환에 성공했습니다."
local MESSAGE_TRADE_FAILED               = "교환에 실패했습니다."

local DEFAULT_INTERVAL = 100

local function tlog(fmt, ...)
    local message = string.format("trade: " .. fmt, ...)
    if message:find("FAILED", 1, true) ~= nil then
        log("fatal", message)
    else
        log("debug", message)
    end
end

local function log_bot(bot, label)
    local pos = bot:position()
    tlog("%s oid=%d map=%d pos=(%d,%d) money=%d",
        label, bot:oid(), bot:map(), pos[1], pos[2], bot:money())
end

local function log_trade_packet(label, packet)
    if packet == nil then
        tlog("%s: packet=nil", label)
        return
    end
    tlog("%s: type=%s dialog_oid=%d dialog_name=%q upload_name=%q money=%d close_message=%q",
        label,
        tostring(packet.type),
        packet.dialog_oid or 0,
        tostring(packet.dialog_name),
        tostring(packet.upload_name),
        packet.money or 0,
        tostring(packet.close_message))
end

local function setup_item(bot, label, item_name, count)
    tlog("%s create_item %s x%d", label, item_name, count)
    bot:create_item(item_name, count)
    tlog("%s create_item %s x%d done, count=%d", label, item_name, count, bot:item_count(item_name))
end

local function setup_money(bot, label, amount)
    tlog("%s money(%d)", label, amount)
    bot:money(amount)
    tlog("%s money done, actual=%d", label, bot:money())
end

test_suite {
    name      = "Trade Test",
    bot_count = 2,

    on_initialize = function(ctx)
        tlog("on_initialize start")
        lib.formation.arrange_in_line(ctx)
        log_bot(ctx:bot(0), "bot1 after formation")
        log_bot(ctx:bot(1), "bot2 after formation")

        local bot2 = ctx:bot(1)
        log_bot(ctx:bot(0), "bot1 after bot2 reposition")
        log_bot(ctx:bot(1), "bot2 after bot2 reposition")
        tlog("on_initialize done")
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            bot:chat("/아이템초기화")
            bot:chat("/아이템삭제")
            bot:money(0)
        end
        ctx:sleep(1000)
    end,

    scenarios = {
        function(ctx)
            tlog("scenario 1 start")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            log_bot(bot1, "bot1")
            log_bot(bot2, "bot2")

            setup_item(bot1, "bot1", "도토리", 150)
            setup_item(bot1, "bot1", "부적", 1)
            setup_money(bot1, "bot1", 10000)

            setup_item(bot2, "bot2", "도토리", 200)
            setup_item(bot2, "bot2", "양첨목봉", 1)
            setup_money(bot2, "bot2", 20000)

            tlog("scenario 1: trade REQUEST bot1 -> bot2 oid=%d", bot2:oid())
            local dialog = trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))
            log_trade_packet("scenario 1: trade REQUEST response", dialog)
            tlog("scenario 1: Bot1 -> Bot2 trade initiated")

            tlog("scenario 1: UP_ITEM charm (slot=%d), waiting message", trade.slot(1))
            local charm = bot1:request(
                resp.message,
                protocol.trade("UP_ITEM", bot2:oid(), { index = trade.slot(1) }),
                function(packet)
                    tlog("scenario 1: charm message validator text=%q type=%s",
                        tostring(packet.text), tostring(packet.type))
                    return string.find(packet.text or "", MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE, 1, true) ~= nil
                end)
            if charm == nil then
                tlog("scenario 1: charm request returned nil")
                return false
            end
            tlog("scenario 1: charm response text=%q", tostring(charm.text))
            tlog("scenario 1: Bot1 tried to trade a charm.")

            tlog("scenario 1: bot1 put_full_offer")
            trade.put_full_offer(bot1, bot2:oid(), 150, 10000)
            tlog("scenario 1: Bot1 puts up items and money.")

            tlog("scenario 1: bot2 put_full_offer")
            trade.put_full_offer(bot2, bot1:oid(), 200, 20000, 1)
            tlog("scenario 1: Bot2 puts up items and money.")

            tlog("scenario 1: bot2 cancel")
            trade.cancel(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_CANCELLED_BY_ME))
            tlog("scenario 1: Bot2 cancelled the trade.")

            tlog("scenario 1: trade REQUEST bot2 -> bot1")
            local dialog2 = trade.request(bot2, bot1:oid(), trade.dialog_oid(bot1:oid()))
            log_trade_packet("scenario 1: trade REQUEST response", dialog2)
            tlog("scenario 1: Bot2 -> Bot1 trade re-initiated.")

            tlog("scenario 1: bot1 put_full_offer (2nd)")
            trade.put_full_offer(bot1, bot2:oid(), 150, 10000)
            tlog("scenario 1: Bot1 puts up items and money again.")

            tlog("scenario 1: bot2 put_full_offer (2nd)")
            trade.put_full_offer(bot2, bot1:oid(), 200, 20000, 1)
            tlog("scenario 1: Bot2 puts up items and money again.")

            tlog("scenario 1: bot1 lock")
            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))
            tlog("scenario 1: Bot1 locked the trade.")

            tlog("scenario 1: bot2 lock (complete)")
            local close_pkt = trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_SUCCESS))
            log_trade_packet("scenario 1: trade complete", close_pkt)
            tlog("scenario 1: Bot2 locked the trade, completing it.")

            ctx:sleep(DEFAULT_INTERVAL)

            tlog("scenario 1 verify: bot1 money=%d bot2 money=%d", bot1:money(), bot2:money())
            tlog("scenario 1 verify: bot1 도토리=%d bot2 도토리=%d",
                bot1:item_count("도토리"), bot2:item_count("도토리"))
            tlog("scenario 1 verify: bot1 양첨목봉=%s bot1 부적=%s",
                tostring(bot1:has_item_by_name("양첨목봉")), tostring(bot1:has_item_by_name("부적")))

            if bot1:money() ~= 20000 or bot2:money() ~= 10000 then
                tlog("scenario 1 FAILED: money mismatch")
                return false
            end
            if bot1:has_item_by_name("양첨목봉") == false then
                tlog("scenario 1 FAILED: bot1 missing 양첨목봉")
                return false
            end
            if bot1:has_item_by_name("부적") == false then
                tlog("scenario 1 FAILED: bot1 missing 부적")
                return false
            end
            if bot1:item_count("도토리") ~= 200 then
                tlog("scenario 1 FAILED: bot1 도토리 count")
                return false
            end
            if bot2:item_count("도토리") ~= 150 then
                tlog("scenario 1 FAILED: bot2 도토리 count")
                return false
            end

            tlog("scenario 1 PASSED")
            return true
        end,

        function(ctx)
            tlog("scenario 2 start")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:money(0xFFFFFFFF)
            bot2:money(1)

            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))
            log("debug", "Scenario 2: Trade initiated for money overflow test.")

            trade.up_money(bot2, bot1:oid(), 1)
            log("debug", "Scenario 2: Bot2 puts up 1 gold.")

            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))
            log("debug", "Scenario 2: Bot1 locked the trade.")

            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            log("debug", "Scenario 2: Bot2 tried to lock, trade failed as expected.")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:money() ~= 0xFFFFFFFF or bot2:money() ~= 1 then
                return false
            end

            tlog("scenario 2 PASSED")
            return true
        end,

        function(ctx)
            tlog("scenario 3 start")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:create_item("도토리", 150)
            bot2:create_item("도토리", 150)

            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))
            log("debug", "Scenario 3: Trade initiated for item stack overflow test.")

            trade.up_item(bot1, bot2:oid(), 0)
            trade.item_count(bot1, bot2:oid(), 150)
            log("debug", "Scenario 3: Bot1 puts up 도토리.")

            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))
            log("debug", "Scenario 3: Bot1 locked the trade.")

            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            log("debug", "Scenario 3: Bot2 tried to lock, trade failed as expected.")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:item_count("도토리") ~= 150 or bot2:item_count("도토리") ~= 150 then
                return false
            end

            tlog("scenario 3 PASSED")
            return true
        end,

        function(ctx)
            tlog("scenario 4 start")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:fill_inventory("목도")
            bot2:create_item("현철중검", 1)
            log("debug", "Scenario 4: Bot1 filled inventory.")

            trade.request(bot1, bot2:oid(), trade.dialog_oid(bot2:oid()))
            log("debug", "Scenario 4: Trade initiated for inventory full test.")

            trade.up_item_upload(bot2, bot1:oid(), 0)
            log("debug", "Scenario 4: Bot2 puts up an item.")

            trade.lock(bot1, bot2:oid(), trade.type_is("lock"))
            log("debug", "Scenario 4: Bot1 locked the trade.")

            trade.lock(bot2, bot1:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            log("debug", "Scenario 4: Bot2 tried to lock, trade failed as expected.")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:has_item_by_name("현철중검") or bot2:has_item_by_name("현철중검") == false then
                return false
            end

            tlog("scenario 4 PASSED")
            return true
        end,

        function(ctx)
            tlog("scenario 5 start")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:fill_inventory("목도")
            bot1:drop_item(0, false)
            bot1:create_item("도토리", 100)
            log("debug", "Scenario 5: Bot1 removed one item and created 100 도토리.")

            bot2:create_item("도토리", 200)
            log("debug", "Scenario 5: Bot2 created 200 도토리.")

            trade.request(bot2, bot1:oid(), trade.dialog_oid(bot1:oid()))
            log("debug", "Scenario 5: Bot2 -> Bot1 trade initiated.")

            trade.up_item(bot2, bot1:oid(), 0)
            trade.item_count(bot2, bot1:oid(), 200)
            log("debug", "Scenario 5: Bot2 puts up 200 도토리.")

            trade.lock(bot2, bot1:oid(), trade.type_is("lock"))
            log("debug", "Scenario 5: Bot2 locked the trade.")

            trade.lock(bot1, bot2:oid(), trade.close_contains(MESSAGE_TRADE_FAILED))
            log("debug", "Scenario 5: Bot1 tried to lock, trade failed as expected.")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:item_count("도토리") ~= 100 or bot2:item_count("도토리") ~= 200 then
                return false
            end

            trade.request(bot2, bot1:oid(), trade.dialog_oid(bot1:oid()))
            log("debug", "Scenario 5: Bot2 -> Bot1 trade re-initiated.")

            trade.up_item(bot2, bot1:oid(), 0)
            trade.item_count(bot2, bot1:oid(), 101)
            log("debug", "Scenario 5: Bot2 puts up 101 도토리.")

            trade.lock(bot2, bot1:oid(), trade.type_is("lock"))
            log("debug", "Scenario 5: Bot2 locked the trade.")

            trade.lock(bot1, bot2:oid(), trade.close_contains(MESSAGE_TRADE_SUCCESS))
            log("debug", "Scenario 5: Bot1 locked the trade, completing it successfully.")

            ctx:sleep(DEFAULT_INTERVAL)

            if bot1:item_count("도토리") ~= 201 then
                return false
            end
            if bot2:item_count("도토리") ~= 99 then
                return false
            end

            tlog("scenario 5 PASSED")
            return true
        end,
    },
}
