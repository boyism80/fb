local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local MESSAGE_ITEM_FULL                  = "더 이상 가질 수 없습니다."
local MESSAGE_MONEY_FULL                 = "더 이상 돈을 가질 수 없습니다."
local MESSAGE_ITEM_DEATH_PENALTY_WARMTH  = "죽은 자의 온기가 남아있습니다."

local DEFAULT_INTERVAL = 100

test_suite {
    name      = "Drop Loot Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Drop loot test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            bot:chat("/아이템삭제")
            bot:chat("/아이템초기화")
            bot:money(0)
        end
    end,

    scenarios = {
        function(ctx)
            local bot1 = ctx:bot(0)
            log("debug", "Starting drop loot scenario 1")

            bot1:create_item("목도", 1)
            bot1:create_item("목검", 1)
            bot1:create_item("양첨목봉", 1)
            bot1:create_item("현철중검", 1)
            bot1:create_item("도토리", 200)
            bot1:money(50000)

            for i = 0, 4 do
                bot1:drop_item(i, true)
            end

            bot1:request(
                resp.update_internal,
                protocol.item_drop_money(50000),
                function(packet)
                    return packet.ch_money == 0
                end)
            if bot1:money() ~= 0 then
                return false
            end

            bot1:request(resp.update_internal, protocol.loot(false), function(packet)
                return packet.ch_money == 50000
            end)
            if bot1:money() ~= 50000 or bot1:inventory_size() ~= 0 then
                return false
            end

            bot1:request(
                resp.item_update,
                protocol.loot(false),
                function(packet)
                    return string.find(packet.name, "도토리", 1, true) ~= nil and packet.count == 200
                end)

            bot1:request(
                resp.item_update,
                protocol.loot(true),
                function(packet)
                    return packet.index == 4
                end)

            if bot1:has_item_by_name("목도") == false
                or bot1:has_item_by_name("목검") == false
                or bot1:has_item_by_name("양첨목봉") == false
                or bot1:has_item_by_name("현철중검") == false then
                return false
            end

            return true
        end,

        function(ctx)
            log("debug", "Starting drop loot scenario 2")
            local bot1 = ctx:bot(0)

            bot1:fill_inventory("목도")
            bot1:drop_item(0, false)
            bot1:create_item("목도", 1)

            bot1:request(
                resp.message,
                protocol.loot(false),
                function(packet)
                    return string.find(packet.text, MESSAGE_ITEM_FULL, 1, true) ~= nil
                end)

            return true
        end,

        function(ctx)
            log("debug", "Starting drop loot scenario 3")
            local bot1 = ctx:bot(0)

            bot1:create_item("도토리", 200)
            local dotori_slot = bot1:item_slot("도토리")
            bot1:drop_item(dotori_slot, true)
            bot1:create_item("도토리", 2)

            bot1:request(
                resp.message,
                protocol.loot(false),
                function(packet)
                    return string.find(packet.text, MESSAGE_ITEM_FULL, 1, true) ~= nil
                end)

            if bot1:item_count("도토리") ~= 201 then
                return false
            end

            return true
        end,

        function(ctx)
            log("debug", "Starting drop loot scenario 4")
            local bot1 = ctx:bot(0)

            bot1:money(2)
            bot1:drop_money(2)
            bot1:money(0xFFFFFFFE)

            bot1:request(
                resp.message,
                protocol.loot(false),
                function(packet)
                    return string.find(packet.text, MESSAGE_MONEY_FULL, 1, true) ~= nil
                end)

            if bot1:money() ~= 0xFFFFFFFF then
                return false
            end

            return true
        end,

        function(ctx)
            log("debug", "Starting drop loot scenario 5")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:learn_spells({ "헬파이어" })
            bot1:setup_bot_stats(100000, 10000)
            bot2:setup_bot_stats(100000, 100000, 50)

            ctx:sleep(DEFAULT_INTERVAL)
            bot2:direction("BOTTOM")

            bot2:create_item("도토리", 200)
            bot2:create_item("목도", 1)
            bot2:create_item("목검", 1)

            local mokdo_slot = bot2:item_slot("목도")
            bot2:request(
                resp.message,
                protocol.item_active(mokdo_slot),
                function(packet)
                    return string.find(packet.text, "w:무기  :", 1, true) == 1
                end)

            local pos = bot2:position()
            bot1:request(
                resp.update_external_detailed,
                protocol.spell_cast("TARGET", 0, "", bot2:oid(), pos),
                function(packet)
                    return packet.oid == bot2:oid() and packet.state == "GHOST"
                end)

            ctx:sleep(500)
            bot2:move("RIGHT")
            bot1:move("RIGHT")

            bot1:request(
                resp.message,
                protocol.loot(false),
                function(packet)
                    return string.find(packet.text, MESSAGE_ITEM_DEATH_PENALTY_WARMTH, 1, true) ~= nil
                end)

            if bot1:inventory_size() > 0 then
                return false
            end

            return true
        end,
    },
}
