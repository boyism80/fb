local resp = require("integration.response")
local protocol = require("integration.protocol")
local script_step = require("integration.lib.script_step")
local quest_dialog = require("integration.lib.quest_dialog")

local BEAST = "백호"
local HEART_MAP = 2041
local THRONE_X = 20
local THRONE_Y = 8
local SEWER_FLOOR_MAP = 2201
local SEWER_FLOOR_X = 38
local SEWER_FLOOR_Y = 6
local MESSAGE_WAIT_MS = 15000

local MSG_PREPARED = "공성준비 완료"
local MSG_STARTED = "공성전을 시작했습니다"
local MSG_ENDED = "공성전 종료: " .. BEAST
local MSG_NOT_ENDED = "진행 중인 공성전이 없습니다."
local MSG_RECAPTURED = "의성을 탈환합니다."
local MSG_DEFENDER = "수성측"
local MSG_EVICTED = "성 내부에서 퇴장되었습니다."

local GATE_NPC = "백호성문지기"
local GATE_OPTION = BEAST .. "성 입장"
local ENTRY_MONEY = 300000
local MSG_FEE_ASK = "입장료를 내고서라도"
local MSG_FEE_PAY = "입장료 20만전"
local MSG_CHANGED = "성의 상황이 바뀌었습니다."
local ENTRANCE_FEE = 200000
local FEE_TO_OWNER = 150000
local MONEY_PATH = "/clan/money"
local ABORT_PATH = "/clan/money/abort"
local ENTER_WAIT_MS = 20000
local SETTLE_WAIT_MS = 45000

local function fail(message)
    log("fatal", "Siege test: " .. message)
    return false
end

local function run_step(bot, func, step, ...)
    return script_step.run_script(bot, "test/integration", func, step, ...) == true
end

local function clan_money(bot, step)
    local ok, text = script_step.run_script(bot, "test/integration", "clan_money", step)
    if ok ~= true or text == nil then
        return nil
    end
    return tonumber(text:match(step .. " (%d+)"))
end

local function clear_fee_faults(bot)
    bot:chat("/HTTP장애 " .. MONEY_PATH .. " off")
    bot:chat("/HTTP장애 " .. ABORT_PATH .. " off")
end

local function command(bot, text, expected)
    local packet = bot:request(resp.message, protocol.chat(false, text), function(p)
        return p.text ~= nil and p.text:find(expected, 1, true) ~= nil
    end, MESSAGE_WAIT_MS)
    return packet ~= nil and packet ~= false
end

test_suite({
    name = "Siege Test",
    bot_count = 2,

    should_skip = function(_ctx)
        if localhost() then
            log("debug", "Siege test skipped on local host (the castle heart map runs on another game server)")
            return true
        end
        return false
    end,

    on_scenario_finished = function(ctx)
        local bot = ctx:bot(0)
        bot:chat("/HTTP장애 /castle/owner off")
        bot:chat("/HTTP지연 internal 0")
        clear_fee_faults(bot)
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

            if
                command(a, "/공성준비", MSG_PREPARED) == false
                or command(b, "/공성준비", MSG_PREPARED) == false
            then
                return fail("could not prepare the clans")
            end

            if command(a, "/공성시작 " .. BEAST, MSG_STARTED) == false then
                return fail("could not start the siege")
            end
            if
                run_step(
                    a,
                    "move_to_map",
                    "siege_a_throne",
                    tostring(HEART_MAP),
                    tostring(THRONE_X),
                    tostring(THRONE_Y)
                ) == false
            then
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
            if
                run_step(
                    a,
                    "move_to_map",
                    "siege_a_aside",
                    tostring(HEART_MAP),
                    tostring(THRONE_X + 2),
                    tostring(THRONE_Y + 2)
                ) == false
            then
                return fail("could not move A off the throne")
            end
            if
                run_step(
                    b,
                    "move_to_map",
                    "siege_b_throne",
                    tostring(HEART_MAP),
                    tostring(THRONE_X),
                    tostring(THRONE_Y)
                ) == false
            then
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
            local ask =
                a:request_dialog(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"), quest_dialog.contains(MSG_FEE_PAY))
            if ask == nil or ask == false then
                return fail("the gate keeper did not ask for the entrance fee")
            end

            if command(b, "/공성시작 " .. BEAST, MSG_STARTED) == false then
                return fail("could not start the siege during the gate dialog")
            end

            local packet = a:request_dialog_ext(
                protocol.dialog("PURSUIT", 0, "", 0, 0, "입장합니다."),
                quest_dialog.contains(MSG_CHANGED)
            )
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

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)

            if run_step(a, "map_active", "fee_heart_map", tostring(HEART_MAP)) == false then
                log("debug", "Siege fee test skipped: the heart map does not run on this host")
                return true
            end
            if run_step(b, "verify_siege", "fee_owned_b", BEAST, "false", "mine") == false then
                return fail("B's clan does not own the castle before the fee check")
            end

            local clan_before = clan_money(b, "fee_clan_before")
            if clan_before == nil then
                return fail("could not read B's clan money")
            end

            a:money(ENTRY_MONEY)
            ctx:sleep(300)
            local gate_map = a:map()
            if quest_dialog.open_pursuit(a, GATE_NPC, GATE_OPTION, quest_dialog.contains(MSG_FEE_ASK)) == nil then
                return fail("the gate keeper did not offer the paid entry")
            end
            local ask =
                a:request_dialog(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"), quest_dialog.contains(MSG_FEE_PAY))
            if ask == nil or ask == false then
                return fail("the gate keeper did not ask for the entrance fee")
            end

            a:chat("/HTTP장애 " .. MONEY_PATH .. " lost")
            a:chat("/HTTP장애 " .. ABORT_PATH .. " lost")
            ctx:sleep(300)
            a:send(protocol.dialog("PURSUIT", 0, "", 0, 0, "입장합니다."))

            local waited = 0
            while a:map() == gate_map and waited < ENTER_WAIT_MS do
                ctx:sleep(500)
                waited = waited + 500
            end
            clear_fee_faults(a)
            if a:map() == gate_map then
                quest_dialog.remove_npc(a)
                return fail("A was kept out while the clan share of the fee was unknown")
            end
            if a:money() ~= ENTRY_MONEY - ENTRANCE_FEE then
                return fail(
                    string.format(
                        "A's money after entering: money=%d expected=%d",
                        a:money(),
                        ENTRY_MONEY - ENTRANCE_FEE
                    )
                )
            end

            local expected = clan_before + FEE_TO_OWNER
            local current = clan_money(b, "fee_clan_wait")
            waited = 0
            while current ~= expected and waited < SETTLE_WAIT_MS do
                ctx:sleep(1000)
                waited = waited + 1000
                current = clan_money(b, "fee_clan_wait")
            end
            if current ~= expected then
                return fail(
                    string.format("B's clan money after the resend: money=%s expected=%d", tostring(current), expected)
                )
            end

            ctx:sleep(3000)
            current = clan_money(b, "fee_clan_after")
            if current ~= expected or a:money() ~= ENTRY_MONEY - ENTRANCE_FEE then
                return fail(
                    string.format(
                        "the fee settled twice: clan=%s expected=%d money=%d",
                        tostring(current),
                        expected,
                        a:money()
                    )
                )
            end
            return true
        end,

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)

            if run_step(a, "map_active", "lost_heart_map", tostring(HEART_MAP)) == false then
                log("debug", "Siege lost save test skipped: the heart map does not run on this host")
                return true
            end
            if run_step(b, "verify_siege", "lost_owned_b", BEAST, "false", "mine") == false then
                return fail("B's clan does not own the castle before the lost save check")
            end

            if command(a, "/공성시작 " .. BEAST, MSG_STARTED) == false then
                return fail("could not start the siege for the lost save check")
            end
            if
                run_step(
                    b,
                    "move_to_map",
                    "lost_b_aside",
                    tostring(HEART_MAP),
                    tostring(THRONE_X + 2),
                    tostring(THRONE_Y + 2)
                ) == false
            then
                return fail("could not move B off the throne")
            end
            if
                run_step(
                    a,
                    "move_to_map",
                    "lost_a_throne",
                    tostring(HEART_MAP),
                    tostring(THRONE_X),
                    tostring(THRONE_Y)
                ) == false
            then
                return fail("could not move A to the throne")
            end

            a:chat("/HTTP장애 /castle/owner lost")
            ctx:sleep(300)
            if command(a, "/공성종료", MSG_NOT_ENDED) == false then
                return fail("the siege ended although the owner response was lost")
            end
            if run_step(a, "verify_siege", "lost_kept", BEAST, "true", "not_mine") == false then
                return fail("the siege did not stay active after the lost response")
            end

            a:chat("/HTTP장애 /castle/owner off")
            ctx:sleep(300)
            if run_step(a, "destroy_clan", "lost_destroy_a") == false then
                return fail("could not destroy A's clan")
            end

            a:take_messages()
            if command(a, "/공성종료", MSG_ENDED) == false then
                return fail("the retried end did not finish the siege after the winner was destroyed")
            end
            for _, text in ipairs(a:take_messages()) do
                if text:find(MSG_DEFENDER, 1, true) ~= nil then
                    return fail("the castle was announced as returned to the defender: " .. text)
                end
            end
            if run_step(b, "verify_siege", "lost_no_owner", BEAST, "false", "none") == false then
                return fail("the siege host still keeps an owner although the stored owner was cleared")
            end
            return true
        end,

        function(ctx)
            local a = ctx:bot(0)

            if run_step(a, "map_active", "sewer_floor_map", tostring(SEWER_FLOOR_MAP)) == false then
                log("debug", "Siege sewer floor test skipped: the sewer floor does not run on this host")
                return true
            end

            a:take_messages()
            run_step(
                a,
                "move_to_map",
                "sewer_floor_enter",
                tostring(SEWER_FLOOR_MAP),
                tostring(SEWER_FLOOR_X),
                tostring(SEWER_FLOOR_Y)
            )
            local evicted = false
            local waited = 0
            while evicted == false and waited < ENTER_WAIT_MS do
                for _, text in ipairs(a:take_messages()) do
                    if text:find(MSG_EVICTED, 1, true) ~= nil then
                        evicted = true
                    end
                end
                if evicted == false then
                    ctx:sleep(500)
                    waited = waited + 500
                end
            end
            if evicted == false or a:map() == SEWER_FLOOR_MAP then
                return fail("a character outside the owner clan stayed on the sewer floor")
            end
            return true
        end,
    },
})
