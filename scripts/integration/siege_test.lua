local resp        = require("integration.response")
local protocol    = require("integration.protocol")
local script_step = require("integration.lib.script_step")
local quest_dialog = require("integration.lib.quest_dialog")

local BEAST      = "백호"
local HEART_MAP  = 2041
local THRONE_X   = 20
local THRONE_Y   = 8
local MESSAGE_WAIT_MS = 15000

local MSG_PREPARED   = "공성준비 완료"
local MSG_STARTED    = "공성전을 시작했습니다"
local MSG_ENDED      = "공성전 종료: " .. BEAST
local MSG_NOT_ENDED  = "진행 중인 공성전이 없습니다."
local MSG_RECAPTURED = "의성을 탈환합니다."

local GATE_NPC      = "백호성문지기"
local GATE_OPTION   = BEAST .. "성 입장"
local ENTRY_MONEY   = 300000
local MSG_FEE_ASK   = "입장료를 내고서라도"
local MSG_FEE_PAY   = "입장료 20만전"
local MSG_CHANGED   = "성의 상황이 바뀌었습니다."

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
        bot:chat("/HTTP장애 /castle/owner off")
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
            if run_step(a, "move_to_map", "siege_a_throne", tostring(HEART_MAP), tostring(THRONE_X), tostring(THRONE_Y)) == false then
                return fail("could not move A to the throne")
            end

            a:chat("/HTTP장애 /castle/owner on")
            ctx:sleep(300)
            a:take_messages()
            if command(a, "/공성종료", MSG_NOT_ENDED) == false then
                return fail("the siege ended although the owner was not saved")
            end
            for _, text in ipairs(a:take_messages()) do
                if text:find(MSG_RECAPTURED, 1, true) ~= nil then
                    return fail("a recapture was announced although the owner was not saved")
                end
            end
            if run_step(a, "verify_siege", "siege_kept", BEAST, "true", "not_mine") == false then
                return fail("the siege did not stay active after the failed save")
            end

            a:chat("/HTTP장애 /castle/owner off")
            ctx:sleep(300)
            if command(a, "/공성종료", MSG_ENDED) == false then
                return fail("the retried end did not finish the siege")
            end
            if run_step(a, "verify_siege", "siege_owned_a", BEAST, "false", "mine") == false then
                return fail("A's clan does not own the castle after the retried end")
            end

            if command(a, "/공성시작 " .. BEAST, MSG_STARTED) == false then
                return fail("could not restart the siege")
            end
            if run_step(a, "move_to_map", "siege_a_aside", tostring(HEART_MAP), tostring(THRONE_X + 2), tostring(THRONE_Y + 2)) == false then
                return fail("could not move A off the throne")
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
            local ended = 0
            for _, text in ipairs(a:take_messages()) do
                if text:find(MSG_RECAPTURED, 1, true) ~= nil then
                    recaptured = recaptured + 1
                end
                if text:find(MSG_ENDED, 1, true) ~= nil then
                    ended = ended + 1
                end
            end
            if recaptured ~= 1 then
                return fail(string.format("the recapture was announced %d times", recaptured))
            end
            if ended ~= 1 then
                return fail(string.format("the ending command answered %d times", ended))
            end
            if run_step(b, "verify_siege", "siege_owned_b", BEAST, "false", "mine") == false then
                return fail("B's clan does not own the castle after the racing ends")
            end

            return true
        end,

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)

            if run_step(a, "map_active", "gate_heart_map", tostring(HEART_MAP)) == false then
                log("debug", "Siege gate test skipped: the heart map does not run on this host")
                return true
            end
            if run_step(b, "verify_siege", "gate_owned_b", BEAST, "false", "mine") == false then
                return fail("B's clan does not own the castle before the gate check")
            end

            a:money(ENTRY_MONEY)
            ctx:sleep(300)
            if quest_dialog.open_pursuit(a, GATE_NPC, GATE_OPTION, quest_dialog.contains(MSG_FEE_ASK)) == nil then
                return fail("the gate keeper did not offer the paid entry")
            end
            local ask = a:request_dialog(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"), quest_dialog.contains(MSG_FEE_PAY))
            if ask == nil or ask == false then
                return fail("the gate keeper did not ask for the entrance fee")
            end

            if command(b, "/공성시작 " .. BEAST, MSG_STARTED) == false then
                return fail("could not start the siege during the gate dialog")
            end

            local packet = a:request_dialog_ext(protocol.dialog("PURSUIT", 0, "", 0, 0, "입장합니다."), quest_dialog.contains(MSG_CHANGED))
            quest_dialog.remove_npc(a)
            if packet == nil then
                return fail("the gate keeper let A in although the siege started")
            end
            ctx:sleep(500)
            if a:money() ~= ENTRY_MONEY then
                return fail(string.format("A paid the entrance fee: money=%d expected=%d", a:money(), ENTRY_MONEY))
            end

            if command(b, "/공성종료", MSG_ENDED) == false then
                return fail("could not end the siege after the gate check")
            end
            return true
        end,
    },
}
