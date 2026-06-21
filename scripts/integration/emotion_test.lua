local resp     = require("integration.response")
local protocol = require("integration.protocol")

local EMOTION_ACTION_BASE = 0x0B

local EMOTIONS = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0xFE, 0xFF,
}

test_suite {
    name      = "Emotion Test",
    bot_count = 1,

    scenarios = {
        function(ctx)
            log("debug", "Starting emotion packet handler test")
            local bot = ctx:bot(0)

            for _, emotion_value in ipairs(EMOTIONS) do
                local expected_action = (EMOTION_ACTION_BASE + emotion_value) % 256
                log("debug", "Testing emotion value: " .. tostring(emotion_value))

                bot:request(
                    resp.action,
                    protocol.emotion(emotion_value),
                    function(packet)
                        return packet.oid == bot:oid()
                            and packet.value_id == expected_action
                    end)
            end

            return true
        end,
    },
}
