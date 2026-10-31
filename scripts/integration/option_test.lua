local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local option      = require("integration.lib.option")
local script_step = require("integration.lib.script_step")

local LABEL_WHISPER = "귓속말듣기"
local LABEL_ROAR    = "외치기듣기"

local function fail(message)
    log("fatal", "Option test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function is_off(label)
    return function(packet)
        return packet.type == "STATE"
            and packet.text:find(label, 1, true) ~= nil
            and packet.text:find(": OFF", 1, true) ~= nil
    end
end

test_suite {
    name      = "Option Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        option.set_option(bot, "WHISPER", true, LABEL_WHISPER)
        option.set_option(bot, "ROAR", true, LABEL_ROAR)
    end,

    scenarios = {
        -- Two options changed back to back must both survive a relogin; a whole-row write loses one of them.
        function(ctx)
            local bot = ctx:bot(0)
            if option.set_option(bot, "WHISPER", true, LABEL_WHISPER) == false then
                return fail("could not turn WHISPER on")
            end
            if option.set_option(bot, "ROAR", true, LABEL_ROAR) == false then
                return fail("could not turn ROAR on")
            end

            for round = 1, 3 do
                bot:send(protocol.update_option("WHISPER", false))
                local roar = bot:request(resp.message, protocol.update_option("ROAR", false), is_off(LABEL_ROAR))
                if roar == false or roar == nil then
                    return fail(string.format("round %d: ROAR did not turn off", round))
                end
                ctx:sleep(500)

                if bot:logout() == false then
                    return fail(string.format("round %d: logout failed", round))
                end
                if bot:login() == false then
                    return fail(string.format("round %d: login failed", round))
                end
                bot = ctx:bot(0)

                if run_step(bot, "verify_option", "whisper_off_" .. round, "WHISPER", "false") == false then
                    return fail(string.format("round %d: WHISPER was not saved off", round))
                end
                if run_step(bot, "verify_option", "roar_off_" .. round, "ROAR", "false") == false then
                    return fail(string.format("round %d: ROAR was not saved off", round))
                end

                if option.set_option(bot, "WHISPER", true, LABEL_WHISPER) == false then
                    return fail(string.format("round %d: could not turn WHISPER back on", round))
                end
                if option.set_option(bot, "ROAR", true, LABEL_ROAR) == false then
                    return fail(string.format("round %d: could not turn ROAR back on", round))
                end
            end
            return true
        end,

        function(ctx)
            local bot = ctx:bot(0)
            local expected = "false"
            if run_step(bot, "verify_option", "helmet_before", "VISIBLE_HELMET", "true") then
                expected = "true"
            end

            local packet = bot:request(resp.message, protocol.update_option("VISIBLE_HELMET", false), function(p)
                return p.type == "STATE" and p.text:find(": O", 1, true) ~= nil
            end)
            if type(packet) == "table" then
                return fail("option 0x0E from a non-6.51 client answered " .. tostring(packet.text))
            end
            if run_step(bot, "verify_option", "helmet_after", "VISIBLE_HELMET", expected) == false then
                return fail("option 0x0E from a non-6.51 client changed the visible helmet setting")
            end
            return true
        end,
    },
}
