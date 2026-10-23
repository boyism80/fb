local resp = require("integration.response")
local protocol = require("integration.protocol")
local script_step = require("integration.lib.script_step")

local MSG_PREPARED = "공성준비 완료"
local MSG_THREAD_ERROR = "active thread not matched"
local OTHER_MAP = 2041
local OTHER_X = 20
local OTHER_Y = 10
local RESPONSE_DELAY = 3000
local MESSAGE_WAIT_MS = 15000

local function fail(message)
    log("fatal", "Special chat thread test: " .. message)
    return false
end

test_suite({
    name = "Special Chat Thread Test",
    bot_count = 1,

    on_scenario_finished = function(ctx)
        ctx:bot(0):chat("/HTTP지연 internal 0")
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)

            local packet = a:request(resp.message, protocol.chat(false, "/공성준비"), function(p)
                return p.text ~= nil and p.text:find(MSG_PREPARED, 1, true) ~= nil
            end, MESSAGE_WAIT_MS)
            if packet == nil or packet == false then
                return fail("could not prepare A's clan")
            end

            a:chat(string.format("/HTTP지연 internal %d", RESPONSE_DELAY))
            ctx:sleep(300)
            a:take_messages()
            a:send(protocol.whisper("!", "special chat thread test"))
            ctx:sleep(300)
            if
                script_step.run_script(
                    a,
                    "test/integration",
                    "move_to_map",
                    "special_chat_move",
                    tostring(OTHER_MAP),
                    tostring(OTHER_X),
                    tostring(OTHER_Y)
                ) ~= true
            then
                return fail("could not move A while the clan broadcast was pending")
            end

            ctx:sleep(RESPONSE_DELAY + 1000)
            a:chat("/HTTP지연 internal 0")
            for _, text in ipairs(a:take_messages()) do
                if text:find(MSG_THREAD_ERROR, 1, true) ~= nil then
                    return fail("the clan chat read the character on a thread it had left")
                end
            end
            return true
        end,
    },
})
