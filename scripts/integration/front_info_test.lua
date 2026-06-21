local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

test_suite {
    name      = "Front Info Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Front info test initialized")
        lib.formation.arrange_in_line(ctx)
    end,

    on_scenario_finished = function(ctx)
        ctx:bot(0):clear_all_drop_items()
    end,

    scenarios = {
        function(ctx)
            log("debug", "Starting front_info packet handler test")
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)

            bot2:create_item("목도", 1)
            bot2:create_item("목검", 1)
            bot2:create_item("도토리", 100)
            bot2:money(10000)

            bot2:drop_item(0, true)
            bot2:drop_item(1, true)
            bot2:drop_item(2, true)
            bot2:drop_money(10000)

            local expected = {
                [bot2:name()] = true,
                ["목도"] = true,
                ["목검"] = true,
                ["도토리 100개"] = true,
                ["금덩어리 10000전"] = true,
            }
            bot1:direction("RIGHT")

            bot1:request(
                resp.message,
                protocol.front_info(),
                function(packet)
                    if packet.type ~= "STATE" then
                        return false
                    end
                    if expected[packet.text] then
                        expected[packet.text] = nil
                    end
                    local remaining = 0
                    for _ in pairs(expected) do
                        remaining = remaining + 1
                    end
                    return remaining == 0
                end)

            return true
        end,
    },
}
