local resp     = require("integration.response")
local protocol = require("integration.protocol")
local model    = require("integration.model")

test_suite {
    name      = "Worldmap Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            log("debug", "Starting worldmap test")
            local bot = ctx:bot(0)

            bot:transfer(protocol.chat(false, "/맵이동 국내성진입로 8 1"))

            local pos = bot:position()
            bot:request(
                resp.map_worlds,
                protocol.move("TOP", bot:oid(), pos),
                function()
                    return true
                end)

            bot:transfer(protocol.map_world(0, 1, 0))

            local dest = model.world_destination(0, 0)
            if bot:map() ~= dest.map then
                return false
            end

            local position = bot:position()
            if position[1] ~= dest.position[1] or position[2] ~= dest.position[2] then
                return false
            end

            return true
        end,
    },
}
