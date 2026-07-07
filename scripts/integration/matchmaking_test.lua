local lib = require("integration.lib")
local mm  = require("integration.lib.matchmaking")

local MESSAGE_WAIT_MS = 10000
local LISTENER_ARM_MS = 500

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("matchmaking_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function bot_at(ctx, index)
    return ctx:bot(index)
end

test_suite {
    name      = "Matchmaking Test",
    bot_count = 6,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "MATCHMAKING TEST INITIALIZED WITH " .. ctx:bot_count() .. " BOTS")
        lib.formation.arrange_in_line(ctx)
    end,

    on_finished = function(ctx)
        lib.group.cleanup(ctx)
    end,

    scenarios = {
        function(ctx)
            local a = bot_at(ctx, 0)
            local b = bot_at(ctx, 1)
            local c = bot_at(ctx, 2)
            local d = bot_at(ctx, 3)
            local e = bot_at(ctx, 4)
            local f = bot_at(ctx, 5)

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
            if mm.f1_fail_not_group_master(b) == false then
                progress(b, "FAILED: B SHOULD BE REJECTED AS NON-GROUP-LEADER")
                return false
            end
            if mm.wait_state_message(ctx, b, mm.MSG_REGISTER_START, 1000) then
                progress(b, "FAILED: B SHOULD NOT RECEIVE REGISTER MESSAGE")
                return false
            end

            progress(a, "STEP 4: LEADERS A AND C REGISTER MATCH 2 VIA F1")
            if mm.f1_register_match2(a) == false then
                progress(a, "FAILED: A F1 REGISTER DIALOG FLOW FAILED")
                return false
            end
            if mm.wait_state_message(ctx, b, mm.MSG_REGISTER_START, MESSAGE_WAIT_MS) == false then
                progress(b, "FAILED: B DID NOT RECEIVE REGISTER MESSAGE")
                return false
            end

            if mm.f1_register_match2(c) == false then
                progress(c, "FAILED: C F1 REGISTER DIALOG FLOW FAILED")
                return false
            end
            if mm.wait_state_message(ctx, d, mm.MSG_REGISTER_START, MESSAGE_WAIT_MS) == false then
                progress(d, "FAILED: D DID NOT RECEIVE REGISTER MESSAGE")
                return false
            end
            if mm.wait_state_message(ctx, e, mm.MSG_REGISTER_START, 1000) then
                progress(e, "FAILED: E SHOULD NOT BE ENROLLED YET")
                return false
            end

            progress(b, "STEP 5: B LEAVES GROUP, AB UNREGISTER")
            local leave_ok = false
            if mm.wait_all_state_messages(ctx, { a, b }, mm.MSG_REGISTER_CANCEL, MESSAGE_WAIT_MS, function()
                leave_ok = b:leave_group()
            end) == false then
                progress(a, "FAILED: AB DID NOT RECEIVE UNREGISTER MESSAGE")
                return false
            end
            if leave_ok == false then
                progress(b, "FAILED: B COULD NOT LEAVE GROUP")
                return false
            end
            if mm.wait_state_message(ctx, a, mm.MSG_REGISTER_START, 1000) then
                progress(a, "FAILED: A SHOULD NOT REMAIN ENROLLED")
                return false
            end
            if mm.wait_state_message(ctx, c, mm.MSG_REGISTER_CANCEL, 1000) then
                progress(c, "FAILED: C SHOULD REMAIN ENROLLED")
                return false
            end
            if mm.wait_state_message(ctx, d, mm.MSG_REGISTER_CANCEL, 1000) then
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
                    local a = bot_at(ctx, 0)
                    progress(a, "STEP 6-PARALLEL: A re-register and confirm")
                    ctx:sleep(LISTENER_ARM_MS)
                    if mm.f1_register_match2(a) == false then
                        progress(a, "FAILED: A COULD NOT RE-REGISTER")
                        return false
                    end
                    if mm.confirm_proposal_dialog(a, ctx) == false then
                        progress(a, "FAILED: A COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if mm.wait_state_message(ctx, a, mm.MSG_MATCH_READY, MESSAGE_WAIT_MS) == false then
                        progress(a, "FAILED: A DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [1] = { function(ctx)
                    local b = bot_at(ctx, 1)
                    progress(b, "STEP 6-PARALLEL: B confirm proposal")
                    if mm.confirm_proposal_dialog(b, ctx) == false then
                        progress(b, "FAILED: B COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if mm.wait_state_message(ctx, b, mm.MSG_MATCH_READY, MESSAGE_WAIT_MS) == false then
                        progress(b, "FAILED: B DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [2] = { function(ctx)
                    local c = bot_at(ctx, 2)
                    progress(c, "STEP 6-PARALLEL: C confirm proposal")
                    if mm.confirm_proposal_dialog(c, ctx) == false then
                        progress(c, "FAILED: C COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if mm.wait_state_message(ctx, c, mm.MSG_MATCH_READY, MESSAGE_WAIT_MS) == false then
                        progress(c, "FAILED: C DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [3] = { function(ctx)
                    local d = bot_at(ctx, 3)
                    progress(d, "STEP 6-PARALLEL: D confirm proposal")
                    if mm.confirm_proposal_dialog(d, ctx) == false then
                        progress(d, "FAILED: D COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if mm.wait_state_message(ctx, d, mm.MSG_MATCH_READY, MESSAGE_WAIT_MS) == false then
                        progress(d, "FAILED: D DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [4] = { function(ctx)
                    local e = bot_at(ctx, 4)
                    progress(e, "STEP 6-PARALLEL: E register and confirm")
                    ctx:sleep(LISTENER_ARM_MS)
                    if mm.f1_register_match2(e) == false then
                        progress(e, "FAILED: E COULD NOT REGISTER")
                        return false
                    end
                    if mm.confirm_proposal_dialog(e, ctx) == false then
                        progress(e, "FAILED: E COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if mm.wait_state_message(ctx, e, mm.MSG_MATCH_READY, MESSAGE_WAIT_MS) == false then
                        progress(e, "FAILED: E DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
                [5] = { function(ctx)
                    local f = bot_at(ctx, 5)
                    progress(f, "STEP 6-PARALLEL: F register and confirm")
                    ctx:sleep(LISTENER_ARM_MS)
                    if mm.f1_register_match2(f) == false then
                        progress(f, "FAILED: F COULD NOT REGISTER")
                        return false
                    end
                    if mm.confirm_proposal_dialog(f, ctx) == false then
                        progress(f, "FAILED: F COULD NOT CONFIRM MATCH PROPOSAL")
                        return false
                    end
                    if mm.wait_state_message(ctx, f, mm.MSG_MATCH_READY, MESSAGE_WAIT_MS) == false then
                        progress(f, "FAILED: F DID NOT RECEIVE MATCH READY MESSAGE")
                        return false
                    end
                    return true
                end },
            },
        },
    },
}
