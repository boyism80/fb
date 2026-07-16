local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local MESSAGE_ITEM_CANNOT_GIVE            = "줄 수 없습니다."
local MESSAGE_ITEM_TARGET_INVENTORY_FULL  = "상대방의 인벤토리가 가득 찼습니다."
local MESSAGE_MONEY_TARGET_CANNOT_RECEIVE = "상대방이 돈을 받을 수 없습니다."

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

test_suite {
    name      = "Item Test Give",
    bot_count = 2,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "ITEM GIVE TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
        progress(ctx:bot(0), "FORMATION DONE")
    end,

    on_scenario_started = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            progress(bot, "CLEANUP INVENTORY")
            bot:clear_all_drop_items()
            bot:clear_inventory()
            bot:money(0)
        end
    end,

    scenarios = {
        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 1 START (cannot give bound item)")

            bot1:direction("RIGHT")
            bot2:direction("LEFT")

            progress(bot1, "CREATE 초심자의노리개")
            bot1:create_item("초심자의노리개", 1)

            progress(bot1, "GIVE ITEM (expect reject)")
            local packet = bot1:request(
                resp.message,
                protocol.give_item(0, false),
                function(p)
                    return p.type == "STATE"
                end)

            if string.find(packet.text, MESSAGE_ITEM_CANNOT_GIVE, 1, true) == nil then
                progress(bot1, "FAILED: expected cannot-give message")
                return false
            end
            progress(bot1, "REJECTED AS EXPECTED")

            if bot1:has_item_by_name("초심자의노리개") == false then
                progress(bot1, "FAILED: item missing after reject")
                return false
            end

            ctx:sleep(DEFAULT_INTERVAL)
            if bot2:has_item_by_name("초심자의노리개") then
                progress(bot2, "FAILED: bot2 received unbound item")
                return false
            end

            progress(bot1, "SCENARIO 1 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 2 START (give wood sword)")

            progress(bot1, "CREATE 목도")
            bot1:create_item("목도", 1)

            progress(bot1, "GIVE 목도 -> bot2")
            bot1:request(
                resp.item_remove,
                protocol.give_item(0, false),
                function(p)
                    return p.index == 0
                end)

            if bot1:has_item_by_name("목도") then
                progress(bot1, "FAILED: still has 목도")
                return false
            end

            ctx:sleep(DEFAULT_INTERVAL)
            if bot2:has_item_by_name("목도") == false then
                progress(bot2, "FAILED: did not receive 목도")
                return false
            end

            progress(bot1, "SCENARIO 2 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 3 START (inventory full)")

            progress(bot2, "FILL INVENTORY 목도")
            bot2:fill_inventory("목도")
            progress(bot1, "CREATE 목도")
            bot1:create_item("목도", 1)

            progress(bot1, "GIVE 목도 (expect inventory full)")
            bot1:request(
                resp.message,
                protocol.give_item(0, false),
                function(p)
                    return p.type == "STATE"
                        and string.find(p.text, MESSAGE_ITEM_TARGET_INVENTORY_FULL, 1, true) ~= nil
                end)
            progress(bot1, "FULL REJECT AS EXPECTED")

            if bot1:has_item_by_name("목도") == false then
                progress(bot1, "FAILED: item lost after full reject")
                return false
            end

            progress(bot2, "DROP ONE SLOT, PREPARE 도토리 STACK")
            bot2:drop_item(0, false)
            bot1:create_item("도토리", 3)
            bot2:create_item("도토리", 200)

            progress(bot1, "GIVE ONE 도토리")
            bot1:request(
                resp.item_update,
                protocol.give_item(1, false),
                function(p)
                    return p.index == 1
                end)

            progress(bot1, "GIVE ALL 도토리 (expect full)")
            bot1:request(
                resp.message,
                protocol.give_item(1, true),
                function(p)
                    return p.type == "STATE"
                        and string.find(p.text, MESSAGE_ITEM_TARGET_INVENTORY_FULL, 1, true) ~= nil
                end)
            progress(bot1, "STACK FULL REJECT AS EXPECTED")

            progress(bot1, "SCENARIO 3 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            progress(bot1, "SCENARIO 4 START (money give / overflow)")

            progress(bot1, "SET MONEY MAX-1 AND GIVE ALL")
            bot1:money(0xFFFFFFFE)

            bot1:request(resp.update_internal, protocol.give_money(0xFFFFFFFE), function(packet)
                return packet.ch_money == 0
            end)
            ctx:sleep(DEFAULT_INTERVAL)
            if bot2:money() ~= 0xFFFFFFFE then
                progress(bot2, "FAILED: bot2 money mismatch after give")
                return false
            end
            progress(bot1, "GAVE ALMOST MAX MONEY")

            progress(bot1, "SET MONEY 2, GIVE UNTIL CAP")
            bot1:money(2)
            bot1:request(resp.update_internal, protocol.give_money(2), function(packet)
                return packet.ch_money == 1
            end)

            progress(bot1, "GIVE AGAIN (expect cannot receive)")
            bot1:request(
                resp.message,
                protocol.give_money(2),
                function(p)
                    return p.type == "STATE"
                        and string.find(p.text, MESSAGE_MONEY_TARGET_CANNOT_RECEIVE, 1, true) ~= nil
                end)
            progress(bot1, "MONEY CAP REJECT AS EXPECTED")

            if bot1:money() ~= 1 or bot2:money() ~= 0xFFFFFFFF then
                progress(bot1, "FAILED: money overflow outcome wrong")
                return false
            end

            progress(bot1, "SCENARIO 4 PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            progress(bot1, "SCENARIO 5 START (give to mob then recover)")

            bot1:direction("BOTTOM")
            progress(bot1, "LEARN 헬파이어 / 노획, CREATE 목도, SET MONEY")
            bot1:learn_spell("헬파이어")
            bot1:learn_spell("노획")
            bot1:create_item("목도", 1)
            bot1:money(1000)
            bot1:set_max_hp_mp(1000, 1000)

            local pos = bot1:position()
            progress(bot1, "SPAWN 다람쥐")
            local mob = bot1:spawn_monster("다람쥐", pos[1], pos[2] + 1)
            if mob.oid == 0 then
                progress(bot1, "FAILED: spawn 다람쥐")
                return false
            end

            progress(bot1, "GIVE 목도 TO MOB")
            bot1:request(
                resp.item_remove,
                protocol.give_item(0, false),
                function(p)
                    return p.index == 0
                end)

            progress(bot1, "GIVE MONEY 500 TO MOB")
            bot1:request(resp.update_internal, protocol.give_money(500), function(packet)
                return packet.ch_money == 500
            end)
            if bot1:money() ~= 500 then
                progress(bot1, "FAILED: money after give to mob")
                return false
            end

            progress(bot1, "CAST 헬파이어")
            bot1:request(
                resp.update,
                protocol.spell_cast("TARGET", 0, "", mob.oid, mob.position),
                function(packet)
                    if packet.object_count < 2 then
                        return false
                    end
                    for _, object in ipairs(packet.objects_data) do
                        if object.x ~= mob.position[1] or object.y ~= mob.position[2] then
                            return false
                        end
                    end
                    return true
                end)

            progress(bot1, "CAST 노획 (recover money)")
            bot1:mp(1000)
            bot1:request(resp.update_internal, protocol.spell_cast("NORMAL", 1, "", mob.oid, mob.position), function(packet)
                return packet.ch_money == 1000
            end)
            if bot1:money() ~= 1000 then
                progress(bot1, "FAILED: money not recovered")
                return false
            end

            ctx:sleep(DEFAULT_INTERVAL)
            if bot1:has_item_by_name("목도") == false then
                progress(bot1, "FAILED: 목도 not recovered")
                return false
            end

            progress(bot1, "SCENARIO 5 PASSED")
            return true
        end,
    },
}
