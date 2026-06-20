local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local MESSAGE_ITEM_CANNOT_GIVE            = "줄 수 없습니다."
local MESSAGE_ITEM_TARGET_INVENTORY_FULL  = "상대방의 인벤토리가 가득 찼습니다."
local MESSAGE_MONEY_TARGET_CANNOT_RECEIVE = "상대방이 돈을 받을 수 없습니다."

local DEFAULT_INTERVAL = 100

test_suite {
    name      = "Item Test Give",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Item give test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_started = function(ctx)
        for i = 0, ctx:bot_count() - 1 do
            local bot = ctx:bot(i)
            bot:clear_all_drop_items()
            bot:clear_inventory()
            bot:money(0)
        end
    end,

    scenarios = {
        function(ctx)
            log("debug", "Starting item give scenario 1")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:direction("RIGHT")
            bot2:direction("LEFT")

            bot1:create_item("초심자의노리개", 1)

            local packet = bot1:request(
                resp.message,
                protocol.give_item(0, false),
                function(p)
                    return p.type == "STATE"
                end)

            if string.find(packet.text, MESSAGE_ITEM_CANNOT_GIVE, 1, true) == nil then
                return false
            end
            if bot1:has_item_by_name("초심자의노리개") == false then
                return false
            end

            ctx:sleep(DEFAULT_INTERVAL)
            if bot2:has_item_by_name("초심자의노리개") then
                return false
            end

            return true
        end,

        function(ctx)
            log("debug", "Starting item give scenario 2")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:create_item("목도", 1)

            bot1:request(
                resp.item_remove,
                protocol.give_item(0, false),
                function(p)
                    return p.index == 0
                end)

            if bot1:has_item_by_name("목도") then
                return false
            end

            ctx:sleep(DEFAULT_INTERVAL)
            if bot2:has_item_by_name("목도") == false then
                return false
            end

            return true
        end,

        function(ctx)
            log("debug", "Starting item give scenario 3")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot2:fill_inventory("목도")
            bot1:create_item("목도", 1)

            bot1:request(
                resp.message,
                protocol.give_item(0, false),
                function(p)
                    return p.type == "STATE"
                        and string.find(p.text, MESSAGE_ITEM_TARGET_INVENTORY_FULL, 1, true) ~= nil
                end)

            if bot1:has_item_by_name("목도") == false then
                return false
            end

            bot2:drop_item(0, false)
            bot1:create_item("도토리", 3)
            bot2:create_item("도토리", 200)

            bot1:request(
                resp.item_update,
                protocol.give_item(1, false),
                function(p)
                    return p.index == 1
                end)

            bot1:request(
                resp.message,
                protocol.give_item(1, true),
                function(p)
                    return p.type == "STATE"
                        and string.find(p.text, MESSAGE_ITEM_TARGET_INVENTORY_FULL, 1, true) ~= nil
                end)

            return true
        end,

        function(ctx)
            log("debug", "Starting item give scenario 4")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:money(0xFFFFFFFE)

            bot1:request(resp.update_internal, protocol.give_money(0xFFFFFFFE), function(packet)
                return packet.ch_money == 0
            end)
            ctx:sleep(DEFAULT_INTERVAL)
            if bot2:money() ~= 0xFFFFFFFE then
                return false
            end

            bot1:money(2)
            bot1:request(resp.update_internal, protocol.give_money(2), function(packet)
                return packet.ch_money == 1
            end)

            bot1:request(
                resp.message,
                protocol.give_money(2),
                function(p)
                    return p.type == "STATE"
                        and string.find(p.text, MESSAGE_MONEY_TARGET_CANNOT_RECEIVE, 1, true) ~= nil
                end)

            if bot1:money() ~= 1 or bot2:money() ~= 0xFFFFFFFF then
                return false
            end

            return true
        end,

        function(ctx)
            log("debug", "Starting item give scenario 5")
            local bot1 = ctx:bot(0)

            bot1:direction("BOTTOM")
            bot1:learn_spell("헬파이어")
            bot1:learn_spell("노획")
            bot1:create_item("목도", 1)
            bot1:money(1000)
            bot1:set_max_hp_mp(1000, 1000)

            local pos = bot1:position()
            local mob = bot1:spawn_monster("다람쥐", pos[1], pos[2] + 1)
            if mob.oid == 0 then
                return false
            end

            bot1:request(
                resp.item_remove,
                protocol.give_item(0, false),
                function(p)
                    return p.index == 0
                end)

            bot1:request(resp.update_internal, protocol.give_money(500), function(packet)
                return packet.ch_money == 500
            end)
            if bot1:money() ~= 500 then
                return false
            end

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

            bot1:mp(1000)
            bot1:request(resp.update_internal, protocol.spell_cast("NORMAL", 1, "", mob.oid, mob.position), function(packet)
                return packet.ch_money == 1000
            end)
            if bot1:money() ~= 1000 then
                return false
            end

            ctx:sleep(DEFAULT_INTERVAL)
            if bot1:has_item_by_name("목도") == false then
                return false
            end

            return true
        end,
    },
}
