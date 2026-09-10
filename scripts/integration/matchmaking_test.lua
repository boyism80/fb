local lib = require("integration.lib")
local resp = require("integration.response")
local protocol = require("integration.protocol")

local TEST_BOTS = 6
local MESSAGE_WAIT_MS = 10000
local TRANSFER_WAIT_MS = 20000
local HOME_WAIT_MS = 20000
local F1_OID = 0xFFFFFFFF
local OPT_MATCHMAKING = "매치메이킹"
local OPT_MATCH_2 = "매치 2"
local OPT_YES = "예"
local REGISTER_CONFIRM_TIMEOUT_MS = 10000
local HOME_MAP = "낙랑의방"

local MSG_NOT_GROUP_MASTER = "그룹장만 매치메이킹을 이용할 수 있습니다."
local MSG_REGISTER_START = "매치메이킹 대기를 시작했습니다"
local MSG_REGISTER_CANCEL = "매치메이킹 대기를 취소했습니다"
local MSG_MATCH_ENDED = "매치가 종료되었습니다."
local MSG_PROPOSAL_DIALOG = "매치를 찾았습니다"

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("matchmaking_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function is_state_text(packet, pattern)
    return packet.type == "STATE"
        and packet.text ~= nil
        and packet.text:find(pattern, 1, true) ~= nil
end

local function f1_register_match2(bot, member)
    bot:request_dialog(
        protocol.click(F1_OID),
        function(packet)
            return packet.type == "pursuit"
        end
    )
    bot:request_dialog(
        protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCHMAKING),
        function(packet)
            return packet.type == "pursuit"
        end
    )
    bot:request_dialog(
        protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_MATCH_2),
        function(packet)
            return packet.type == "pursuit"
        end
    )

    local yes = protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_YES)
    local packet
    if member ~= nil then
        packet = bot:request_on(
            member,
            resp.message,
            yes,
            function(p)
                return is_state_text(p, MSG_REGISTER_START)
            end,
            REGISTER_CONFIRM_TIMEOUT_MS
        )
    else
        packet = bot:request(
            resp.message,
            yes,
            function(p)
                return is_state_text(p, MSG_REGISTER_START)
            end,
            REGISTER_CONFIRM_TIMEOUT_MS
        )
    end

    return packet ~= nil and packet ~= false
end

local function f1_fail_not_group_master(bot)
    bot:request_dialog(
        protocol.click(F1_OID),
        function(packet)
            return packet.type == "pursuit"
        end
    )

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

    -- After NEXT, server.lua loops back to F1 with me:pursuit (0x2F), not 0x30.
    bot:request_dialog(
        protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"),
        function(p)
            return p.type == "pursuit"
        end
    )
    return ok
end

local function confirm_proposal_and_transfer(bot)
    local packet = bot:request_dialog_ext(
        protocol.chat(false, "."),
        function(p)
            return p.type == "list"
                and p.message ~= nil
                and p.message:find(MSG_PROPOSAL_DIALOG, 1, true) ~= nil
        end
    )
    if packet == nil or packet.message == nil then
        return false
    end

    bot:transfer(protocol.dialog("LIST", 0, "", 1, 0, "", "NEXT"))
    return true
end

local function kill_self(bot)
    if bot:state() == "GHOST" then
        return true
    end

    if lib.option.disable_pk_protect(bot) == false then
        return false
    end

    bot:setup_bot_stats(100000, 100000)
    bot:set_current_hp_mp(50, bot:mp())

    local hell_slot = bot:learn_spell("헬파이어")
    if hell_slot == 0xFF then
        return false
    end

    local pos = bot:position()
    local packet = bot:request(
        resp.update_internal,
        protocol.spell_cast("TARGET", hell_slot, "", bot:oid(), pos),
        function(p)
            return p.ch_hp == 0
        end
    )
    return packet ~= nil and packet ~= false
end

test_suite {
    name = "Matchmaking Test",
    bot_count = TEST_BOTS,

    on_initialize = function(ctx)
        progress(ctx:bot(0), "MATCHMAKING TEST INITIALIZED WITH " .. ctx:bot_count() .. " BOTS")
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

            progress(a, "STEP 4: LEADERS A AND C REGISTER MATCH 2 VIA F1")
            if f1_register_match2(a, b) == false then
                progress(a, "FAILED: A F1 REGISTER DIALOG FLOW FAILED")
                return false
            end

            if f1_register_match2(c, d) == false then
                progress(c, "FAILED: C F1 REGISTER DIALOG FLOW FAILED")
                return false
            end

            progress(b, "STEP 5: B LEAVES GROUP, AB UNREGISTER")
            local cancel_packet = b:request_on(
                a,
                resp.message,
                protocol.group(b:name()),
                function(p)
                    return is_state_text(p, MSG_REGISTER_CANCEL)
                end,
                MESSAGE_WAIT_MS
            )
            if cancel_packet == nil or cancel_packet == false then
                progress(a, "FAILED: AB DID NOT RECEIVE UNREGISTER MESSAGE")
                return false
            end

            progress(a, "STEP 6: RE-FORM AB AND MATCH ALL 6 PLAYERS")
            local join = a:request(
                resp.message,
                protocol.group(b:name()),
                function(p)
                    return p.type == "STATE"
                        and p.text ~= nil
                        and p.text:find("님 그룹에 참여") ~= nil
                end
            )
            if join == nil or join == false then
                progress(a, "FAILED TO RE-FORM GROUP AB")
                return false
            end

            return true
        end,

        {
            parallel = {
                [0] = { function(ctx)
                    local a = ctx:bot(0)
                    progress(a, "STEP 6-PARALLEL: A re-register and transfer")
                    if f1_register_match2(a, ctx:bot(1)) == false then
                        progress(ctx:bot(0), "FAILED: A COULD NOT RE-REGISTER")
                        return false
                    end
                    if confirm_proposal_and_transfer(ctx:bot(0)) == false then
                        progress(ctx:bot(0), "FAILED: A DID NOT TRANSFER TO MATCH SERVER")
                        return false
                    end
                    return true
                end },
                [1] = { function(ctx)
                    progress(ctx:bot(1), "STEP 6-PARALLEL: B confirm and transfer")
                    if confirm_proposal_and_transfer(ctx:bot(1)) == false then
                        progress(ctx:bot(1), "FAILED: B DID NOT TRANSFER TO MATCH SERVER")
                        return false
                    end
                    return true
                end },
                [2] = { function(ctx)
                    progress(ctx:bot(2), "STEP 6-PARALLEL: C confirm and transfer")
                    if confirm_proposal_and_transfer(ctx:bot(2)) == false then
                        progress(ctx:bot(2), "FAILED: C DID NOT TRANSFER TO MATCH SERVER")
                        return false
                    end
                    return true
                end },
                [3] = { function(ctx)
                    progress(ctx:bot(3), "STEP 6-PARALLEL: D confirm and transfer")
                    if confirm_proposal_and_transfer(ctx:bot(3)) == false then
                        progress(ctx:bot(3), "FAILED: D DID NOT TRANSFER TO MATCH SERVER")
                        return false
                    end
                    return true
                end },
                [4] = { function(ctx)
                    local e = ctx:bot(4)
                    progress(e, "STEP 6-PARALLEL: E register and transfer")
                    if f1_register_match2(e) == false then
                        progress(ctx:bot(4), "FAILED: E COULD NOT REGISTER")
                        return false
                    end
                    if confirm_proposal_and_transfer(e) == false then
                        progress(ctx:bot(4), "FAILED: E DID NOT TRANSFER TO MATCH SERVER")
                        return false
                    end
                    return true
                end },
                [5] = { function(ctx)
                    local f = ctx:bot(5)
                    progress(f, "STEP 6-PARALLEL: F register and transfer")
                    if f1_register_match2(f) == false then
                        progress(ctx:bot(5), "FAILED: F COULD NOT REGISTER")
                        return false
                    end
                    if confirm_proposal_and_transfer(f) == false then
                        progress(ctx:bot(5), "FAILED: F DID NOT TRANSFER TO MATCH SERVER")
                        return false
                    end
                    return true
                end },
            },
        },

        function(ctx)
            local bots = {}
            for i = 0, TEST_BOTS - 1 do
                bots[#bots + 1] = ctx:bot(i)
            end

            progress(bots[1], "STEP 7: MATCH STARTED ON CROSS, FINISH MATCH AND RETURN HOME")
            for _, bot in ipairs(bots) do
                local model = id2map(bot:map())
                local name = nil
                if model ~= nil then
                    name = model:name()
                end
                if name == nil or name == HOME_MAP then
                    progress(bot, "FAILED: EXPECTED CROSS MATCH MAP, GOT " .. tostring(name))
                    return false
                end
            end

            for i = 1, #bots - 1 do
                if kill_self(bots[i]) == false then
                    progress(bots[i], "FAILED: COULD NOT ENTER GHOST WITH 헬파이어")
                    return false
                end
            end

            local last = bots[#bots]
            if kill_self(last) == false then
                progress(last, "FAILED: COULD NOT ENTER GHOST WITH 헬파이어")
                return false
            end
            local ended = last:request(
                resp.message,
                nil,
                function(p)
                    return is_state_text(p, MSG_MATCH_ENDED)
                end,
                TRANSFER_WAIT_MS
            )
            if ended == nil or ended == false then
                progress(last, "FAILED: DID NOT RECEIVE MATCH ENDED")
                return false
            end

            local waited_ms = 0
            local interval_ms = 200
            local all_home = false
            while waited_ms < HOME_WAIT_MS do
                all_home = true
                for i = 0, TEST_BOTS - 1 do
                    local model = id2map(ctx:bot(i):map())
                    local name = nil
                    if model ~= nil then
                        name = model:name()
                    end
                    if name ~= HOME_MAP then
                        all_home = false
                        break
                    end
                end
                if all_home then
                    break
                end
                ctx:sleep(interval_ms)
                waited_ms = waited_ms + interval_ms
            end
            if all_home == false then
                progress(ctx:bot(0), "FAILED: DID NOT RETURN TO " .. HOME_MAP)
                return false
            end

            progress(ctx:bot(0), "STEP 7 PASSED: MATCH ENDED AND RETURNED HOME")
            return true
        end,
    },
}
