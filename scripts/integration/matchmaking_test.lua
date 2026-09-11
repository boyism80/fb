local lib = require("integration.lib")
local resp = require("integration.response")
local protocol = require("integration.protocol")

local TEST_BOTS = 6
local MAX_HP = 50
local MAX_MP = 10000

local DIALOG_WAIT_MS = 15000
local MESSAGE_WAIT_MS = 15000
local PROPOSAL_WAIT_MS = 25000
local MATCH_END_WAIT_MS = 30000
local TRANSFER_WAIT_MS = 30000
local HOME_WAIT_MS = 30000
local POLL_INTERVAL_MS = 200

-- The bot that triggers a proposal waits this long so every other bot has its
-- proposal hook installed first. A pushed dialog is lost if nobody is listening.
local SETTLE_MS = 2000

local F1_OID = 0xFFFFFFFF
local OPT_MATCHMAKING = "매치메이킹"
local OPT_MATCH_1 = "매치 1"
local OPT_MATCH_2 = "매치 2"
local OPT_YES = "예"
local HOME_MAP = "낙랑의방"

local MSG_NOT_GROUP_MASTER = "그룹장만 매치메이킹을 이용할 수 있습니다."
local MSG_REGISTER_START = "매치메이킹 대기를 시작했습니다"
local MSG_REGISTER_CANCEL = "매치메이킹 대기를 취소했습니다"
local MSG_MATCH_CANCELLED = "매치가 취소되었습니다"
local MSG_MATCH_ENDED = "매치가 종료되었습니다."
local MSG_PROPOSAL_DIALOG = "매치를 찾았습니다"
local MSG_GROUP_JOINED = "님 그룹에 참여"

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    local name = "nil"
    if bot ~= nil then
        name = bot:name() or "nil"
    end
    log(level, string.format("matchmaking_test bot=%s %s", name, message))
    if bot ~= nil then
        bot:chat("=== " .. message .. " ===")
    end
end

local function is_state_text(packet, pattern)
    return packet.type == "STATE"
        and packet.text ~= nil
        and packet.text:find(pattern, 1, true) ~= nil
end

local function map_name(bot)
    local model = id2map(bot:map())
    if model == nil then
        return nil
    end
    return model:name()
end

local function pursuit(bot, packet)
    return bot:request_dialog(
        packet,
        function(p)
            return p.type == "pursuit"
        end,
        DIALOG_WAIT_MS
    )
end

local function f1_register(bot, option, member)
    if pursuit(bot, protocol.click(F1_OID)) == nil then
        progress(bot, "FAILED: F1 MENU DID NOT OPEN")
        return false
    end
    if pursuit(bot, protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCHMAKING)) == nil then
        progress(bot, "FAILED: MATCHMAKING MENU DID NOT OPEN")
        return false
    end
    if pursuit(bot, protocol.dialog("PURSUIT", 0, "", 0, 0, option)) == nil then
        progress(bot, "FAILED: " .. option .. " CONFIRM DID NOT OPEN")
        return false
    end

    local yes = protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_YES)
    local started = function(p)
        return is_state_text(p, MSG_REGISTER_START)
    end

    local packet
    if member ~= nil then
        packet = bot:request_on(member, resp.message, yes, started, MESSAGE_WAIT_MS)
    else
        packet = bot:request(resp.message, yes, started, MESSAGE_WAIT_MS)
    end

    if packet == nil or packet == false then
        progress(bot, "FAILED: NO REGISTER START MESSAGE FOR " .. option)
        return false
    end
    return true
end

local function f1_fail_not_group_master(bot)
    if pursuit(bot, protocol.click(F1_OID)) == nil then
        return false
    end

    local packet = bot:request_dialog_ext(
        protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCHMAKING),
        function(p)
            return p.type == "normal"
                and p.message ~= nil
                and p.message:find(MSG_NOT_GROUP_MASTER, 1, true) ~= nil
        end,
        DIALOG_WAIT_MS
    )
    if packet == nil then
        return false
    end

    -- After NEXT, server.lua loops back to F1 with me:pursuit (0x2F), not 0x30.
    return pursuit(bot, protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT")) ~= nil
end

local function confirm_proposal_and_transfer(bot)
    local packet = bot:request_dialog_ext(
        protocol.chat(false, "."),
        function(p)
            return p.type == "list"
                and p.message ~= nil
                and p.message:find(MSG_PROPOSAL_DIALOG, 1, true) ~= nil
        end,
        PROPOSAL_WAIT_MS
    )
    if packet == nil then
        progress(bot, "FAILED: NO MATCH PROPOSAL DIALOG")
        return false
    end

    bot:transfer(protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"))
    return true
end

local function poll_until(ctx, condition, timeout_ms)
    local waited_ms = 0
    while waited_ms < timeout_ms do
        if condition() then
            return true
        end

        ctx:sleep(POLL_INTERVAL_MS)
        waited_ms = waited_ms + POLL_INTERVAL_MS
    end
    return false
end

local function revive(bot)
    -- /체력바꾸기 clears GHOST even when max hp is unchanged; setup_bot_stats skips
    -- that command when base_hp already matches. Hellfire damage is mp*1.5, so mp
    -- must be restored too — MATCH_2 leaves everyone at 0 mp.
    local hp = bot:request(
        resp.update_internal,
        protocol.chat(false, string.format("/체력바꾸기 %d", MAX_HP)),
        function(p)
            return p.ch_hp ~= nil and p.ch_hp > 0
        end,
        MESSAGE_WAIT_MS
    )
    if hp == nil or hp == false then
        return false
    end

    local mp = bot:request(
        resp.update_internal,
        protocol.chat(false, string.format("/마력바꾸기 %d", MAX_MP)),
        function(p)
            return p.ch_mp ~= nil and p.ch_mp > 0
        end,
        MESSAGE_WAIT_MS
    )
    return mp ~= nil and mp ~= false
end

local function hellfire_cast(bot)
    bot:set_current_hp_mp(MAX_HP, MAX_MP)

    if lib.option.disable_pk_protect(bot) == false then
        return nil
    end

    local slot = bot:learn_spell("헬파이어")
    if slot == 0xFF then
        return nil
    end

    return protocol.spell_cast("TARGET", slot, "", bot:oid(), bot:position())
end

-- bot_indices are suite indices (0-based). Always re-read ctx:bot after any
-- transfer — Lua-held bot refs go stale when the bot reconnects.
-- fatal_pos is 1-based into bot_indices: that bot's death ends the match.
-- Do not wait for "match started" here: on_playing fires during join/transfer,
-- before this scenario can install a hook, so the message is often already gone.
local function finish_match(ctx, bot_indices, fatal_pos)
    local function live(pos)
        return ctx:bot(bot_indices[pos])
    end

    local on_match_map = poll_until(ctx, function()
        for pos = 1, #bot_indices do
            local name = map_name(live(pos))
            if name == nil or name == HOME_MAP then
                return false
            end
        end
        return true
    end, TRANSFER_WAIT_MS)

    if on_match_map == false then
        progress(live(1), "FAILED: EXPECTED MATCH MAP, GOT " .. tostring(map_name(live(1))))
        return false
    end

    for pos = 1, fatal_pos - 1 do
        local bot = live(pos)
        local cast = hellfire_cast(bot)
        if cast == nil then
            progress(bot, "FAILED: COULD NOT PREPARE 헬파이어")
            return false
        end

        local dead = bot:request(
            resp.update_internal,
            cast,
            function(p)
                return p.ch_hp == 0
            end,
            MESSAGE_WAIT_MS
        )
        if dead == nil or dead == false then
            progress(bot, "FAILED: COULD NOT ENTER GHOST WITH 헬파이어")
            return false
        end
    end

    local fatal = live(fatal_pos)
    local cast = hellfire_cast(fatal)
    if cast == nil then
        progress(fatal, "FAILED: COULD NOT PREPARE 헬파이어")
        return false
    end

    local ended = fatal:request(
        resp.message,
        cast,
        function(p)
            return is_state_text(p, MSG_MATCH_ENDED)
        end,
        MATCH_END_WAIT_MS
    )
    if ended == nil or ended == false then
        progress(live(fatal_pos), "FAILED: DID NOT RECEIVE MATCH ENDED")
        return false
    end

    local at_home = poll_until(ctx, function()
        for pos = 1, #bot_indices do
            if map_name(live(pos)) ~= HOME_MAP then
                return false
            end
        end
        return true
    end, HOME_WAIT_MS)

    if at_home == false then
        progress(live(1), "FAILED: DID NOT RETURN TO " .. HOME_MAP)
        return false
    end
    return true
end

local function confirm_lanes(indices, trigger)
    local lanes = {}
    for _, index in ipairs(indices) do
        lanes[index] = { function(ctx)
            local bot = ctx:bot(index)
            progress(bot, "WAIT FOR PROPOSAL AND TRANSFER")
            return confirm_proposal_and_transfer(bot)
        end }
    end

    lanes[trigger.index] = { function(ctx)
        local bot = ctx:bot(trigger.index)
        progress(bot, "SETTLE THEN REGISTER " .. trigger.option)
        bot:sleep(SETTLE_MS)
        if f1_register(bot, trigger.option) == false then
            return false
        end
        return confirm_proposal_and_transfer(bot)
    end }

    return { parallel = lanes }
end

test_suite {
    name = "Matchmaking Test",
    bot_count = TEST_BOTS,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "MATCHMAKING TEST INITIALIZED WITH " .. ctx:bot_count() .. " BOTS")
        for i = 0, ctx:bot_count() - 1 do
            ctx:bot(i):setup_bot_stats(MAX_HP, MAX_MP)
        end
        lib.formation.arrange_in_line(ctx)
    end,

    on_finished = function(ctx)
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

            progress(a, "STEP 4: LEADER A REGISTERS MATCH 2 VIA F1")
            if f1_register(a, OPT_MATCH_2, b) == false then
                return false
            end

            progress(b, "STEP 5: B LEAVES GROUP, AB UNREGISTER")
            local cancelled = b:request_on(
                a,
                resp.message,
                protocol.group(b:name()),
                function(p)
                    return is_state_text(p, MSG_REGISTER_CANCEL)
                        or is_state_text(p, MSG_MATCH_CANCELLED)
                end,
                MESSAGE_WAIT_MS
            )
            if cancelled == nil or cancelled == false then
                progress(a, "FAILED: AB DID NOT RECEIVE UNREGISTER OR MATCH-CANCEL MESSAGE")
                return false
            end

            progress(a, "STEP 6: RE-FORM AB THEN REGISTER FIVE PLAYERS")
            local joined = a:request(
                resp.message,
                protocol.group(b:name()),
                function(p)
                    return is_state_text(p, MSG_GROUP_JOINED)
                end,
                MESSAGE_WAIT_MS
            )
            if joined == nil or joined == false then
                progress(a, "FAILED TO RE-FORM GROUP AB")
                return false
            end

            -- MATCH_2 is 3v3. Five entries cannot form a match, so the proposal
            -- only fires once F registers from inside the parallel block.
            if f1_register(a, OPT_MATCH_2, b) == false then
                return false
            end
            if f1_register(c, OPT_MATCH_2, d) == false then
                return false
            end
            if f1_register(e, OPT_MATCH_2) == false then
                return false
            end

            return true
        end,

        confirm_lanes({0, 1, 2, 3, 4}, {index = 5, option = OPT_MATCH_2}),

        function(ctx)
            local indices = {}
            for i = 0, TEST_BOTS - 1 do
                indices[#indices + 1] = i
            end

            progress(ctx:bot(0), "STEP 7: FINISH MATCH 2 AND RETURN HOME")
            if finish_match(ctx, indices, #indices) == false then
                return false
            end

            progress(ctx:bot(0), "STEP 7 PASSED: MATCH 2 ENDED AND RETURNED HOME")
            return true
        end,

        function(ctx)
            local e = ctx:bot(4)
            local f = ctx:bot(5)

            progress(e, "STEP 8: REVIVE SOLO BOTS THEN REGISTER MATCH 1")
            if revive(e) == false or revive(f) == false then
                progress(e, "FAILED: COULD NOT REVIVE SOLO BOTS")
                return false
            end

            -- MATCH_1 is 1v1, so one entry waits until F registers in parallel.
            return f1_register(e, OPT_MATCH_1)
        end,

        confirm_lanes({4}, {index = 5, option = OPT_MATCH_1}),

        function(ctx)
            progress(ctx:bot(4), "STEP 9: FINISH MATCH 1 AND RETURN HOME")
            -- A single death wipes a 1v1 team, so the first kill ends the match.
            if finish_match(ctx, {4, 5}, 1) == false then
                return false
            end

            progress(ctx:bot(4), "STEP 9 PASSED: MATCH 1 ENDED AND RETURNED HOME")
            return true
        end,
    },
}
