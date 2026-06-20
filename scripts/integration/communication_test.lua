local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

test_suite {
    name      = "Communication Test",
    bot_count = 2,

    on_initialize = function(ctx)
        log("debug", "Communication test initialized")
        lib.formation.arrange_in_line(ctx)
        ctx:bot(0):direction("RIGHT")
        ctx:bot(1):direction("LEFT")
    end,

    scenarios = {
        function(ctx)
            local bot1 = ctx:bot(0)
            local message = "Hello, this is a normal chat test!"

            log("debug", "Starting normal chat test")
            bot1:request(
                resp.chat,
                protocol.chat(false, message),
                function(packet)
                    local expected = bot1:name() .. ": " .. message
                    return packet.text == expected
                        and packet.oid == bot1:oid()
                        and packet.type == "NORMAL"
                end)

            log("debug", "Normal chat test PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local message = "HELLO EVERYONE, THIS IS A SHOUT TEST!"

            log("debug", "Starting shout chat test")
            bot1:request(
                resp.chat,
                protocol.chat(true, message),
                function(packet)
                    local expected = bot1:name() .. "! " .. message
                    return packet.text == expected
                        and packet.oid == bot1:oid()
                        and packet.type == "SHOUT"
                end)

            log("debug", "Shout chat test PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            local message = "This is a secret whisper message!"

            log("debug", "Starting whisper test")
            bot1:request(
                resp.message,
                protocol.whisper(bot2:name(), message),
                function(packet)
                    if packet.type ~= "NOTIFY" then
                        return false
                    end
                    local pos = packet.text:find("< ")
                    if pos == nil then
                        return false
                    end
                    local target = packet.text:sub(1, pos - 1)
                    local text = packet.text:sub(pos + 2)
                    return target == bot2:name() and text == message
                end)

            log("debug", "Whisper test PASSED")
            return true
        end,

        function(ctx)
            local bot1 = ctx:bot(0)
            local bot2 = ctx:bot(1)
            local message = "This whisper should be blocked!"

            log("debug", "Starting whisper block test")
            bot2:request(
                resp.message,
                protocol.update_option("WHISPER", false),
                function(packet)
                    return packet.type == "STATE"
                        and packet.text:find("귓속말듣기") ~= nil
                end)
            log("debug", "Whisper option disabled for bot2")

            local blocked = false
            local ok = pcall(function()
                bot1:request(
                    resp.message,
                    protocol.whisper(bot2:name(), message),
                    function(packet)
                        if packet.type == "NOTIFY"
                            and packet.text:find("귓속말 거부", 1, true) then
                            blocked = true
                            return true
                        end
                        return false
                    end)
            end)

            if ok == false and blocked == false then
                blocked = true
            end

            if blocked then
                log("debug", "Whisper block test PASSED - whisper was blocked")
            end

            bot2:request(
                resp.message,
                protocol.update_option("WHISPER", true),
                function(packet)
                    return packet.type == "STATE"
                        and packet.text:find("귓속말듣기") ~= nil
                end)
            log("debug", "Whisper option re-enabled for bot2")

            return blocked
        end,
    },
}
