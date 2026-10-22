local resp     = require("integration.response")
local protocol = require("integration.protocol")

local F1_OID              = 0xFFFFFFFF
local OPT_MATCHMAKING     = "매치메이킹"
local OPT_MATCH_1         = "매치 1"
local OPT_YES             = "예"
local MSG_REGISTER_START  = "매치메이킹 대기를 시작했습니다"
local MSG_REGISTER_CANCEL = "매치메이킹 대기를 취소했습니다"
local MATCHMAKING_SERVICE = "matchmaking"
-- Long enough for the new enqueue to reach the service before the leftover cleanup sends its dequeue.
local RESPONSE_DELAY_MS   = 3000
local DIALOG_WAIT_MS      = 30000
local MESSAGE_WAIT_MS     = 15000

local function fail(message)
    log("fatal", "Matchmaking dequeue test: " .. message)
    return false
end

local function is_pursuit(p)
    return p.type == "pursuit"
end

local function register(bot)
    if bot:request_dialog(protocol.click(F1_OID), is_pursuit, DIALOG_WAIT_MS) == nil then
        return false
    end
    if bot:request_dialog(protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCHMAKING), is_pursuit, DIALOG_WAIT_MS) == nil then
        return false
    end
    if bot:request_dialog(protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCH_1), is_pursuit, DIALOG_WAIT_MS) == nil then
        return false
    end
    local packet = bot:request(resp.message, protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_YES), function(p)
        return p.type == "STATE" and p.text ~= nil and p.text:find(MSG_REGISTER_START, 1, true) ~= nil
    end, MESSAGE_WAIT_MS)
    return packet ~= nil and packet ~= false
end

test_suite {
    name      = "Matchmaking Dequeue Test",
    bot_count = 2,

    on_scenario_finished = function(ctx)
        local b = ctx:bot(1)
        b:chat("/HTTP장애 " .. MATCHMAKING_SERVICE .. " off")
        b:chat(string.format("/HTTP지연 %s 0", MATCHMAKING_SERVICE))
    end,

    scenarios = {
        -- The login cleanup of a leftover ticket must not remove the ticket registered while it was waiting.
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)

            if register(a) == false then
                return fail("the first registration did not start")
            end

            -- The logout dequeue never arrives, so the service keeps the ticket as a leftover.
            a:chat("/HTTP장애 " .. MATCHMAKING_SERVICE .. " on")
            if a:logout() == false then
                return fail("logout")
            end
            b:chat("/HTTP장애 " .. MATCHMAKING_SERVICE .. " off")
            b:chat(string.format("/HTTP지연 %s %d", MATCHMAKING_SERVICE, RESPONSE_DELAY_MS))
            ctx:sleep(300)

            if a:login() == false then
                return fail("relogin")
            end
            a = ctx:bot(0)
            a:take_messages()
            if register(a) == false then
                return fail("the second registration did not start")
            end

            ctx:sleep(RESPONSE_DELAY_MS * 2)
            for _, text in ipairs(a:take_messages()) do
                if text:find(MSG_REGISTER_CANCEL, 1, true) ~= nil then
                    return fail("the leftover cleanup cancelled the new registration")
                end
            end

            b:chat(string.format("/HTTP지연 %s 0", MATCHMAKING_SERVICE))
            if a:logout() == false or a:login() == false then
                return fail("cleanup relogin")
            end
            return true
        end,
    },
}
