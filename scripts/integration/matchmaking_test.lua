local lib = require("integration.lib")
local resp = require("integration.response")
local protocol = require("integration.protocol")
local bot_diag = require("integration.lib.bot_diag")

local TEST_BOTS = 6
local MAX_HP = 50
local MAX_MP = 10000

local DIALOG_WAIT_MS = 30000
local MESSAGE_WAIT_MS = 15000
local PROPOSAL_WAIT_MS = 45000
local MATCH_END_WAIT_MS = 30000
local TRANSFER_WAIT_MS = 45000
local HOME_WAIT_MS = 45000
local POLL_INTERVAL_MS = 200

local SETTLE_MS = 4000

local aborted = false

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
        if level == "fatal" then
            bot_diag.dump(bot, "matchmaking:" .. message)
        end
    end
end

local function abort_remaining(bot, message)
    aborted = true
    progress(bot, message)
    return false
end

local function if_aborted()
    return aborted
end

local function dump_match_bots(ctx, bot_indices, label)
    for pos = 1, #bot_indices do
        local bot = ctx:bot(bot_indices[pos])
        bot_diag.dump(bot, string.format("%s idx=%s pos=%d", label, tostring(bot_indices[pos]), pos))
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

local function is_proposal_list(packet)
    return packet.type == "list"
        and packet.message ~= nil
        and packet.message:find(MSG_PROPOSAL_DIALOG, 1, true) ~= nil
end

local function f1_register_await_proposal(bot, option)
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
    local caught, packet = pcall(function()
        return bot:request_dialog_ext(yes, is_proposal_list, PROPOSAL_WAIT_MS)
    end)
    if caught == false or packet == nil or packet == false then
        progress(bot, "FAILED: NO MATCH PROPOSAL LIST AFTER REGISTER " .. option)
        return false
    end
    return true
end

local function f1_fail_not_group_master(bot)
    if pursuit(bot, protocol.click(F1_OID)) == nil then
        return false
    end

    local caught, packet = pcall(function()
        return bot:request_dialog_ext(
            protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCHMAKING),
            function(p)
                return p.type == "normal"
                    and p.message ~= nil
                    and p.message:find(MSG_NOT_GROUP_MASTER, 1, true) ~= nil
            end,
            DIALOG_WAIT_MS
        )
    end)
    if caught == false or packet == nil then
        return false
    end

    return pursuit(bot, protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT")) ~= nil
end

local function confirm_proposal_and_transfer(bot)
    local caught, packet = pcall(function()
        return bot:request_dialog_ext(protocol.chat(false, "."), is_proposal_list, PROPOSAL_WAIT_MS)
    end)
    if caught == false or packet == nil or packet == false then
        return abort_remaining(bot, "FAILED: NO MATCH PROPOSAL LIST")
    end

    local ok, err = pcall(function()
        bot:transfer(protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"))
    end)
    if ok == false then
        return abort_remaining(bot, "FAILED: CONFIRM/TRANSFER err=" .. tostring(err))
    end
    return true
end

local function confirm_after_proposal_list(bot)
    local ok, err = pcall(function()
        bot:transfer(protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"))
    end)
    if ok == false then
        progress(bot, "FAILED: CONFIRM/TRANSFER err=" .. tostring(err))
        return false
    end
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
        local maps = {}
        for pos = 1, #bot_indices do
            maps[#maps + 1] = string.format("%d=%s", bot_indices[pos], tostring(map_name(live(pos))))
        end
        progress(live(1), "FAILED: EXPECTED MATCH MAP, GOT " .. table.concat(maps, ", "))
        dump_match_bots(ctx, bot_indices, "matchmaking:not_on_match_map")
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
        dump_match_bots(ctx, bot_indices, "matchmaking:no_match_ended")
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
        local maps = {}
        for pos = 1, #bot_indices do
            maps[#maps + 1] = string.format("%d=%s", bot_indices[pos], tostring(map_name(live(pos))))
        end
        progress(live(1), "FAILED: DID NOT RETURN TO " .. HOME_MAP .. " maps=" .. table.concat(maps, ", "))
        dump_match_bots(ctx, bot_indices, "matchmaking:not_home")
        return false
    end
    return true
end

local function confirm_lanes(indices, trigger)
    local lanes = {}
    for _, index in ipairs(indices) do
        lanes[index] = { function(ctx)
            if if_aborted() then
                return false
            end
            local bot = ctx:bot(index)
            progress(bot, "WAIT FOR PROPOSAL AND TRANSFER")
            return confirm_proposal_and_transfer(bot)
        end }
    end

    lanes[trigger.index] = { function(ctx)
        if if_aborted() then
            return false
        end
        local bot = ctx:bot(trigger.index)
        progress(bot, "SETTLE THEN REGISTER " .. trigger.option)
        bot:sleep(SETTLE_MS)
        if f1_register_await_proposal(bot, trigger.option) == false then
            return abort_remaining(bot, "FAILED: REGISTER/PROPOSAL " .. trigger.option)
        end
        progress(bot, "CONFIRM PROPOSAL AND TRANSFER")
        return confirm_after_proposal_list(bot)
    end }

    return { parallel = lanes }
end

test_suite {
    name = "Matchmaking Test",
    bot_count = TEST_BOTS,

    on_initialize = function(ctx)
        aborted = false
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
            if if_aborted() then
                return false
            end

            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local c = ctx:bot(2)
            local d = ctx:bot(3)
            local e = ctx:bot(4)

            progress(a, "STEP 1-2: FORM GROUPS AB AND CD")
            if a:invite_group(b) == false then
                return abort_remaining(a, "FAILED TO FORM GROUP AB")
            end
            if c:invite_group(d) == false then
                return abort_remaining(c, "FAILED TO FORM GROUP CD")
            end

            progress(b, "STEP 3: NON-LEADER B TRIES F1 MATCHMAKING")
            if f1_fail_not_group_master(b) == false then
                return abort_remaining(b, "FAILED: B SHOULD BE REJECTED AS NON-GROUP-LEADER")
            end

            progress(a, "STEP 4: LEADER A REGISTERS MATCH 2 VIA F1")
            if f1_register(a, OPT_MATCH_2, b) == false then
                return abort_remaining(a, "FAILED: REGISTER MATCH 2")
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
                return abort_remaining(a, "FAILED: AB DID NOT RECEIVE UNREGISTER OR MATCH-CANCEL MESSAGE")
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
                return abort_remaining(a, "FAILED TO RE-FORM GROUP AB")
            end

            if f1_register(a, OPT_MATCH_2, b) == false then
                return abort_remaining(a, "FAILED: RE-REGISTER MATCH 2 AB")
            end
            if f1_register(c, OPT_MATCH_2, d) == false then
                return abort_remaining(c, "FAILED: REGISTER MATCH 2 CD")
            end
            if f1_register(e, OPT_MATCH_2) == false then
                return abort_remaining(e, "FAILED: REGISTER MATCH 2 E")
            end

            return true
        end,

        confirm_lanes({0, 1, 2, 3, 4}, {index = 5, option = OPT_MATCH_2}),

        function(ctx)
            if if_aborted() then
                return false
            end

            local indices = {}
            for i = 0, TEST_BOTS - 1 do
                indices[#indices + 1] = i
            end

            progress(ctx:bot(0), "STEP 7: FINISH MATCH 2 AND RETURN HOME")
            if finish_match(ctx, indices, #indices) == false then
                return abort_remaining(ctx:bot(0), "FAILED: MATCH 2 FINISH")
            end

            progress(ctx:bot(0), "STEP 7 PASSED: MATCH 2 ENDED AND RETURNED HOME")
            return true
        end,

        function(ctx)
            if if_aborted() then
                return false
            end

            local e = ctx:bot(4)
            local f = ctx:bot(5)

            progress(e, "STEP 8: REVIVE SOLO BOTS AND CLEAR GROUPS")
            if revive(e) == false or revive(f) == false then
                return abort_remaining(e, "FAILED: COULD NOT REVIVE SOLO BOTS")
            end

            lib.group.cleanup(ctx)
            return true
        end,

        { parallel = {
            [4] = { function(ctx)
                if if_aborted() then
                    return false
                end
                local bot = ctx:bot(4)
                progress(bot, "REGISTER MATCH 1 THEN CONFIRM")
                if f1_register_await_proposal(bot, OPT_MATCH_1) == false then
                    return abort_remaining(bot, "FAILED: MATCH 1 REGISTER/PROPOSAL E")
                end
                progress(bot, "CONFIRM PROPOSAL AND TRANSFER")
                return confirm_after_proposal_list(bot)
            end },
            [5] = { function(ctx)
                if if_aborted() then
                    return false
                end
                local bot = ctx:bot(5)
                progress(bot, "SETTLE THEN REGISTER MATCH 1")
                bot:sleep(SETTLE_MS)
                if f1_register_await_proposal(bot, OPT_MATCH_1) == false then
                    return abort_remaining(bot, "FAILED: MATCH 1 REGISTER/PROPOSAL F")
                end
                progress(bot, "CONFIRM PROPOSAL AND TRANSFER")
                return confirm_after_proposal_list(bot)
            end },
        } },

        function(ctx)
            if if_aborted() then
                return false
            end

            progress(ctx:bot(4), "STEP 9: FINISH MATCH 1 AND RETURN HOME")
            if finish_match(ctx, {4, 5}, 1) == false then
                return abort_remaining(ctx:bot(4), "FAILED: MATCH 1 FINISH")
            end

            progress(ctx:bot(4), "STEP 9 PASSED: MATCH 1 ENDED AND RETURNED HOME")
            return true
        end,
    },
}
