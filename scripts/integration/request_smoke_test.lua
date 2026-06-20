local resp     = require("integration.response")
local protocol = require("integration.protocol")

test_suite {
    name      = "Request Smoke Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            log("debug", "Request smoke test scenario running")
            local bot = ctx:bot(0)

            local response = bot:request(
                resp.action,
                protocol.attack(),
                function(packet)
                    return packet.oid == bot:oid() and packet.value == "ATTACK"
                end)

            return response ~= nil and response.value == "ATTACK"
        end,
    },
}
