local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local DEFAULT_INTERVAL = 100

test_suite {
    name      = "Throw Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Throw test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        ctx:bot(0):chat("/아이템삭제")
        ctx:sleep(500)
    end,

    scenarios = {
        function(ctx)
            log("debug", "Starting throw scenario 1")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot1:create_item("도토리", 201)
            ctx:sleep(DEFAULT_INTERVAL)

            local pos = bot1:position()
            bot1:request(
                resp.item_throws,
                protocol.item_throws(false, 0),
                function(packet)
                    if packet.ch_oid ~= bot1:oid() then
                        return false
                    end
                    if packet.from_x ~= pos[1] or packet.to_x ~= pos[1] then
                        return false
                    end
                    if packet.from_y ~= pos[2] or packet.to_y ~= pos[2] + 7 then
                        return false
                    end
                    return packet.look == 49366
                end)

            if bot1:item_count("도토리") ~= 200 then
                return false
            end

            return true
        end,

        function(ctx)
            log("debug", "Starting throw scenario 2")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot2:move("BOTTOM", 4)
            bot2:move("LEFT", 1)
            ctx:sleep(500)
            bot2:direction("BOTTOM")

            local from = bot1:position()
            local to   = bot2:position()
            bot1:request(
                resp.item_throws,
                protocol.item_throws(true, 0),
                function(packet)
                    if packet.ch_oid ~= bot1:oid() then
                        return false
                    end
                    if packet.from_x ~= from[1] or packet.to_x ~= to[1] then
                        return false
                    end
                    if packet.from_y ~= from[2] or packet.to_y ~= to[2] - 1 then
                        return false
                    end
                    return packet.look == 49366
                end)

            if bot1:inventory_size() ~= 0 then
                return false
            end

            return true
        end,
    },
}
