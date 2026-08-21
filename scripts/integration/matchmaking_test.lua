local lib = require("integration.lib")
local resp = require("integration.response")
local protocol = require("integration.protocol")

local MESSAGE_WAIT_MS = 10000
local LISTENER_ARM_MS = 500
local F1_OID = 0xFFFFFFFF
local OPT_MATCHMAKING = "매치메이킹"
local OPT_MATCH_2 = "매치 2"
local OPT_YES = "예"
local REGISTER_CONFIRM_TIMEOUT_MS = 10000

local MSG_NOT_GROUP_MASTER = "그룹장만 매치메이킹을 이용할 수 있습니다."
local MSG_REGISTER_START = "매치메이킹 대기를 시작했습니다"
local MSG_REGISTER_CANCEL = "매치메이킹 대기를 취소했습니다"
local MSG_MATCH_CONFIRM = "매치 참여를 수락했습니다"
local MSG_MATCH_READY = "매치가 성사되었습니다"
local MSG_PROPOSAL_DIALOG = "매치를 찾았습니다"

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("matchmaking_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

-- After NEXT on the non-leader reject dialog, server.lua loops back to F1
-- with me:pursuit (0x2F), not another 0x30.
local function dismiss_normal_dialog(bot)
    bot:request_dialog(
        protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "pursuit"
        end
    )
end

local function f1_open_menu(bot)
    bot:request_dialog(
        protocol.click(F1_OID),
        function(packet)
            return packet.type == "pursuit"
        end
    )
end

local function f1_select_matchmaking(bot)
    f1_open_menu(bot)
    bot:request_dialog(
        protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCHMAKING),
        function(packet)
            return packet.type == "pursuit"
        end
    )
end

local function f1_register_match2(bot)
    f1_select_matchmaking(bot)

    bot:request_dialog(
        protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCH_2),
        function(packet)
            return packet.type == "pursuit"
        end
    )

    local packet = bot:request(
        resp.message,
        protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_YES),
        function(p)
            return p.type == "STATE"
                and p.text ~= nil
                and p.text:find(MSG_REGISTER_START, 1, true) ~= nil
        end,
        REGISTER_CONFIRM_TIMEOUT_MS
    )

    return packet ~= nil
end

local function f1_fail_not_group_master(bot)
    f1_open_menu(bot)

    local packet = bot:request_dialog_ext(
        protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCHMAKING),
        function(p)
            return p.type == "normal"
                and p.message ~= nil
                and p.message:find(MSG_NOT_GROUP_MASTER, 1, true) ~= nil
        end
    )

    local ok = packet ~= nil
        and packet.message ~= nil
        and packet.message:find(MSG_NOT_GROUP_MASTER, 1, true) ~= nil

    dismiss_normal_dialog(bot)
    return ok
end

local function wait_proposal_dialog(bot)
    return bot:request_dialog_ext(
        protocol.chat(false, "."),
        function(p)
            return p.type == "list"
                and p.message ~= nil
                and p.message:find(MSG_PROPOSAL_DIALOG, 1, true) ~= nil
        end
    )
end

local function confirm_proposal_dialog(bot)
    local packet = wait_proposal_dialog(bot)
    if packet == nil or packet.message == nil then
        return false
    end

    local confirm_packet = bot:request(
        resp.message,
        protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"),
        function(p)
            return p.type == "STATE"
                and p.text ~= nil
                and p.text:find(MSG_MATCH_CONFIRM, 1, true) ~= nil
        end,
        REGISTER_CONFIRM_TIMEOUT_MS
    )

    return confirm_packet ~= nil
end

local function wait_state_message(ctx, bot, pattern, timeout_ms)
    local found = false
    ctx:hook("message", function(_, hooked_bot, packet)
        if hooked_bot:name() ~= bot:name() then
            return
        end
        if packet.type == "STATE"
            and packet.text ~= nil
            and packet.text:find(pattern, 1, true) ~= nil then
            found = true
        end
    end)

    local waited_ms = 0
    local interval_ms = 200
    while found == false and waited_ms < timeout_ms do
        ctx:sleep(interval_ms)
        waited_ms = waited_ms + interval_ms
    end

    ctx:unhook("message")
    return found
end

local function wait_all_state_messages(ctx, bots, pattern, timeout_ms, after_hook)
    local received = {}
    for _, bot in ipairs(bots) do
        received[bot:name()] = false
    end

    ctx:hook("message", function(_, hooked_bot, packet)
        if packet.type ~= "STATE" or packet.text == nil then
            return
        end
        if packet.text:find(pattern, 1, true) == nil then
            return
        end
        if received[hooked_bot:name()] ~= nil then
            received[hooked_bot:name()] = true
        end
    end)

    if after_hook ~= nil then
        after_hook()
    end

    local waited_ms = 0
    local interval_ms = 200
    while waited_ms < timeout_ms do
        local all_received = true
        for _, bot in ipairs(bots) do
            if received[bot:name()] ~= true then
                all_received = false
                break
            end
        end

        if all_received then
            ctx:unhook("message")
            return true
        end

        ctx:sleep(interval_ms)
        waited_ms = waited_ms + interval_ms
    end

    ctx:unhook("message")
    return false
end

-- Parallel lanes must share one MATCH READY hook. ctx:unhook("message") clears
-- every hook for that opcode, so per-lane wait_state_message races and drops
-- READY notifications for bots that have not finished waiting yet.
local match_ready = {
    armed = false,
    flags = nil,
}

local function match_ready_reset()
    match_ready.armed = false
    match_ready.flags = nil
end

local function match_ready_arm(ctx, bots)
    if match_ready.armed then
        return true
    end

    match_ready.flags = {}
    for _, bot in ipairs(bots) do
        match_ready.flags[bot:name()] = false
    end

    ctx:hook("message", function(_, hooked_bot, packet)
        if packet.type ~= "STATE" or packet.text == nil then
            return
        end
        if packet.text:find(MSG_MATCH_READY, 1, true) == nil then
            return
        end
        if match_ready.flags[hooked_bot:name()] ~= nil then
            match_ready.flags[hooked_bot:name()] = true
        end
    end)

    match_ready.armed = true
    return true
end

local function match_ready_wait_armed(ctx, timeout_ms)
    local waited_ms = 0
    local interval_ms = 50
    while match_ready.armed == false and waited_ms < timeout_ms do
        ctx:sleep(interval_ms)
        waited_ms = waited_ms + interval_ms
    end
    return match_ready.armed
end

local function match_ready_wait(ctx, bot, timeout_ms)
    local waited_ms = 0
    local interval_ms = 200
    while waited_ms < timeout_ms do
        if match_ready.flags ~= nil and match_ready.flags[bot:name()] == true then
            return true
        end
        ctx:sleep(interval_ms)
        waited_ms = waited_ms + interval_ms
    end
    return match_ready.flags ~= nil and match_ready.flags[bot:name()] == true
end

local function match_ready_wait_all(ctx, bots, timeout_ms)
    local waited_ms = 0
    local interval_ms = 200
    while waited_ms < timeout_ms do
        local all_received = true
        for _, bot in ipairs(bots) do
            if match_ready.flags == nil or match_ready.flags[bot:name()] ~= true then
                all_received = false
                break
            end
        end
        if all_received then
            return true
        end
        ctx:sleep(interval_ms)
        waited_ms = waited_ms + interval_ms
    end
    return false
end

local function match_ready_disarm(ctx)
    if match_ready.armed then
        ctx:unhook("message")
        match_ready.armed = false
    end
end

test_suite {
    name = "Matchmaking Test",
    bot_count = 6,

    on_initialize = function(ctx)
        match_ready_reset()
        progress(ctx:bot(0), "MATCHMAKING TEST INITIALIZED WITH " .. ctx:bot_count() .. " BOTS")
        lib.formation.arrange_in_line(ctx)
    end,

    on_finished = function(ctx)
        match_ready_disarm(ctx)
        match_ready_reset()
        lib.group.cleanup(ctx)
    end,

    scenarios = {
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local c = ctx:bot(2)
            local d = ctx:bot(3)
            local e = ctx:bot(4)

            progress(a, "STEP 1-2: FORM GROUPS AB AND CD")
            if a:invite_group(b) == false then
                progress(a, "FAILED TO FORM GROUP AB")
                return false
            end
            if c:invite_group(d) == false then
                progress(c, "FAILED TO FORM GROUP CD")
                return false
            end

            progress(b, "STEP 3: NON-LEADER B TRIES F1 MATCHMAKING")
            if f1_fail_not_group_master(b) == false then
                progress(b, "FAILED: B SHOULD BE REJECTED AS NON-GROUP-LEADER")
                return false
            end
            if wait_state_message(ctx, b, MSG_REGISTER_START, 1000) then
                progress(b, "FAILED: B SHOULD NOT RECEIVE REGISTER MESSAGE")
                return false
            end

            progress(a, "STEP 4: LEADERS A AND C REGISTER MATCH 2 VIA F1")
            if f1_register_match2(a) == false then
                progress(a, "FAILED: A F1 REGISTER DIALOG FLOW FAILED")
                return false
            end
            if wait_state_message(ctx, b, MSG_REGISTER_START, MESSAGE_WAIT_MS) == false then
                progress(b, "FAILED: B DID NOT RECEIVE REGISTER MESSAGE")
                return false
            end

            if f1_register_match2(c) == false then
                progress(c, "FAILED: C F1 REGISTER DIALOG FLOW FAILED")
                return false
            end
            if wait_state_message(ctx, d, MSG_REGISTER_START, MESSAGE_WAIT_MS) == false then
                progress(d, "FAILED: D DID NOT RECEIVE REGISTER MESSAGE")
                return false
            end
            if wait_state_message(ctx, e, MSG_REGISTER_START, 1000) then
                progress(e, "FAILED: E SHOULD NOT BE ENROLLED YET")
                return false
            end

            progress(b, "STEP 5: B LEAVES GROUP, AB UNREGISTER")
            local leave_ok = false
            if wait_all_state_messages(ctx, { a, b }, MSG_REGISTER_CANCEL, MESSAGE_WAIT_MS, function()
                leave_ok = b:leave_group()
            end) == false then
                progress(a, "FAILED: AB DID NOT RECEIVE UNREGISTER MESSAGE")
                return false
            end
            if leave_ok == false then
                progress(b, "FAILED: B COULD NOT LEAVE GROUP")
                return false
            end
            if wait_state_message(ctx, a, MSG_REGISTER_START, 1000) then
                progress(a, "FAILED: A SHOULD NOT REMAIN ENROLLED")
                return false
            end
            if wait_state_message(ctx, c, MSG_REGISTER_CANCEL, 1000) then
                progress(c, "FAILED: C SHOULD REMAIN ENROLLED")
                return false
            end
            if wait_state_message(ctx, d, MSG_REGISTER_CANCEL, 1000) then
                progress(d, "FAILED: D SHOULD REMAIN ENROLLED")
                return false
            end

            progress(a, "STEP 6: RE-FORM AB AND MATCH ALL 6 PLAYERS")
            if a:invite_group(b) == false then
                progress(a, "FAILED TO RE-FORM GROUP AB")
                return false
            end

            return true
        end,

        {
            parallel = {
                [0] = { function(ctx)
                    local a = ctx:bot(0)
                    local bots = {
                        ctx:bot(0),
                        ctx:bot(1),
                        ctx:bot(2),
                        ctx:bot(3),
                        ctx:bot(4),
                        ctx:bot(5),
                    }
                    progress(a, "STEP 6-PARALLEL: A re-register and confirm")
                    match_ready_arm(ctx, bots)
                    ctx:sleep(LISTENER_ARM_MS)

                    local ok = false
                    if f1_register_match2(a) == false then
                        progress(a, "FAILED: A COULD NOT RE-REGISTER")
                    elseif confirm_proposal_dialog(a) == false then
                        progress(a, "FAILED: A COULD NOT CONFIRM MATCH PROPOSAL")
                    else
                        -- Hold the shared hook until every lane has seen READY (or timeout).
                        match_ready_wait_all(ctx, bots, MESSAGE_WAIT_MS)
                        if match_ready_wait(ctx, a, 0) == false then
                            progress(a, "FAILED: A DID NOT RECEIVE MATCH READY MESSAGE")
                        else
                            ok = true
                        end
                    end

                    match_ready_disarm(ctx)
                    return ok
                end },
                [1] = { function(ctx)
                    local b = ctx:bot(1)
                    progress(b, "STEP 6-PARALLEL: B confirm proposal")
                    if match_ready_wait_armed(ctx, MESSAGE_WAIT_MS) == false then
                        progress(b, "FAILED: MATCH READY HOOK NOT ARMED")
                        return false
                    end
                    if confirm_proposal_dialog(b) == false then
                        progress(b, "FAILED: B COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if match_ready_wait(ctx, b, MESSAGE_WAIT_MS) == false then
                        progress(b, "FAILED: B DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [2] = { function(ctx)
                    local c = ctx:bot(2)
                    progress(c, "STEP 6-PARALLEL: C confirm proposal")
                    if match_ready_wait_armed(ctx, MESSAGE_WAIT_MS) == false then
                        progress(c, "FAILED: MATCH READY HOOK NOT ARMED")
                        return false
                    end
                    if confirm_proposal_dialog(c) == false then
                        progress(c, "FAILED: C COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if match_ready_wait(ctx, c, MESSAGE_WAIT_MS) == false then
                        progress(c, "FAILED: C DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [3] = { function(ctx)
                    local d = ctx:bot(3)
                    progress(d, "STEP 6-PARALLEL: D confirm proposal")
                    if match_ready_wait_armed(ctx, MESSAGE_WAIT_MS) == false then
                        progress(d, "FAILED: MATCH READY HOOK NOT ARMED")
                        return false
                    end
                    if confirm_proposal_dialog(d) == false then
                        progress(d, "FAILED: D COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if match_ready_wait(ctx, d, MESSAGE_WAIT_MS) == false then
                        progress(d, "FAILED: D DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [4] = { function(ctx)
                    local e = ctx:bot(4)
                    progress(e, "STEP 6-PARALLEL: E register and confirm")
                    if match_ready_wait_armed(ctx, MESSAGE_WAIT_MS) == false then
                        progress(e, "FAILED: MATCH READY HOOK NOT ARMED")
                        return false
                    end
                    ctx:sleep(LISTENER_ARM_MS)
                    if f1_register_match2(e) == false then
                        progress(e, "FAILED: E COULD NOT REGISTER")
                        return false
                    end
                    if confirm_proposal_dialog(e) == false then
                        progress(e, "FAILED: E COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if match_ready_wait(ctx, e, MESSAGE_WAIT_MS) == false then
                        progress(e, "FAILED: E DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [5] = { function(ctx)
                    local f = ctx:bot(5)
                    progress(f, "STEP 6-PARALLEL: F register and confirm")
                    if match_ready_wait_armed(ctx, MESSAGE_WAIT_MS) == false then
                        progress(f, "FAILED: MATCH READY HOOK NOT ARMED")
                        return false
                    end
                    ctx:sleep(LISTENER_ARM_MS)
                    if f1_register_match2(f) == false then
                        progress(f, "FAILED: F COULD NOT REGISTER")
                        return false
                    end
                    if confirm_proposal_dialog(f) == false then
                        progress(f, "FAILED: F COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if match_ready_wait(ctx, f, MESSAGE_WAIT_MS) == false then
                        progress(f, "FAILED: F DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
            },
        },
    },
}
