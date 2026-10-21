local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local BEAST      = "백호"
local HEART_MAP  = 2041
local THRONE_X   = 20
local THRONE_Y   = 8
local MESSAGE_WAIT_MS = 15000

local MSG_PREPARED   = "공성준비 완료"
local MSG_STARTED    = "공성전을 시작했습니다"
local MSG_RECAPTURED = "의성을 탈환합니다."

local function fail(message)
    log("fatal", "Siege test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function command(bot, text, expected)
    local packet = bot:request(resp.message, protocol.chat(false, text), function(p)
        return p.text ~= nil and p.text:find(expected, 1, true) ~= nil
    end, MESSAGE_WAIT_MS)
    return packet ~= nil and packet ~= false
end

test_suite {
    name      = "Siege Test",
    bot_count = 2,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        bot:chat("/HTTP지연 internal 0")
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)

            if run_step(a, "map_active", "siege_heart_map", tostring(HEART_MAP)) == false then
                log("debug", "Siege test skipped: the heart map does not run on this host")
                return true
            end

            if command(a, "/공성종료", "공성전") == false then
                return fail("could not end the sieges left by other suites")
            end

            if command(a, "/공성준비", MSG_PREPARED) == false or command(b, "/공성준비", MSG_PREPARED) == false then
                return fail("could not prepare the clans")
            end

            if command(a, "/공성시작 " .. BEAST, MSG_STARTED) == false then
                return fail("could not start the siege")
            end
            if run_step(b, "move_to_map", "siege_b_throne", tostring(HEART_MAP), tostring(THRONE_X), tostring(THRONE_Y)) == false then
                return fail("could not move B to the throne")
            end

            a:chat("/HTTP지연 internal 1500")
            ctx:sleep(300)
            a:take_messages()
            a:send(protocol.chat(false, "/공성종료"))
            a:send(protocol.chat(false, "/공성종료"))
            ctx:sleep(5000)
            a:chat("/HTTP지연 internal 0")

            local recaptured = 0
            for _, text in ipairs(a:take_messages()) do
                if text:find(MSG_RECAPTURED, 1, true) ~= nil then
                    recaptured = recaptured + 1
                end
            end
            if recaptured ~= 1 then
                return fail(string.format("the recapture was announced %d times", recaptured))
            end
            if run_step(b, "verify_siege", "siege_owned_b", BEAST, "false", "mine") == false then
                return fail("B's clan does not own the castle after the racing ends")
            end

            return true
        end,

    },
}
