local lib      = require("integration.lib")
local resp     = require("integration.response")
local protocol = require("integration.protocol")

local AGENT_NPC  = "문파대리인"
local SENSEI_NPC = "문파성선생"

local DIALOG_CLOSE_OID = 0xFFFFFFFD
local DIALOG_NEXT      = 2

local CAT_OPERATION = 2
local OP_DESTROY    = 6
local CONFIRM_YES   = 1

local OPT_VAULT    = "문파자금"
local OPT_DEPOSIT  = "문파자금 입금"
local OPT_WITHDRAW = "문파자금 출금"

local MSG_CREATE_OK   = "축하하오"
local MSG_DESTROY_OK  = "문파 해체 수속이 처리되었소"
local MSG_DEPOSIT_OK  = "문파자금에 입금했습니다"
local MSG_DEPOSIT_NO  = "문파자금 입금에 실패했습니다"
local MSG_WITHDRAW_OK = "잘 쓰시게나"
local MSG_UNKNOWN     = "결과를 확인하지 못했습니다"

local MONEY_PATH = "/clan/money"
local ABORT_PATH = "/clan/money/abort"

local START_MONEY = 1000
local AMOUNT      = 100

local MQ_WAIT_MS = 2000
-- unsettled_settle_timer runs every 30s.
local SETTLE_FIRST_WAIT_MS = 30000
local SETTLE_POLL_MS       = 5000
local SETTLE_POLL_COUNT    = 8

local g_sensei = nil

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("clan_money_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function clear_faults(bot)
    bot:chat("/HTTP장애 " .. MONEY_PATH .. " off")
    bot:chat("/HTTP장애 " .. ABORT_PATH .. " off")
    bot:chat("/HTTP보류 off")
end

local function close_dialog(bot)
    bot:send(protocol.click(DIALOG_CLOSE_OID))
end

-- The final me:dialog has no follow-up packet, so do not wait for one.
local function dismiss_normal(bot)
    bot:send(protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"))
end

local function spawn_npc(bot, name)
    bot:direction("BOTTOM")
    bot:move("BOTTOM")
    local npc = bot:create_npc(name)
    bot:move("TOP")
    bot:direction("BOTTOM")
    return npc
end

local function remove_npc(bot, npc)
    bot:request(resp.hide, protocol.chat(false, "/엔피씨제거"), function(packet)
        return packet.oid == npc.oid
    end)
end

local function select_list(bot, index, expect_type)
    local use_ext = (expect_type == "list" or expect_type == "normal")
    local fn = use_ext and bot.request_dialog_ext or bot.request_dialog
    return fn(bot,
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == expect_type
        end)
end

local function create_clan(bot, npc)
    local packet = bot:request_dialog_ext(protocol.click(npc.oid), function(p)
        return p.type == "list"
    end)
    if packet == nil then
        return nil, "create confirm missing"
    end
    packet = select_list(bot, CONFIRM_YES, "input")
    if packet == nil then
        return nil, "create input missing"
    end
    packet = bot:request_dialog_ext(protocol.dialog("INPUT", 0, bot:name(), 0, 0, "", "NEXT"), function(p)
        return p.type == "normal"
    end)
    if packet == nil then
        return nil, "create result missing"
    end
    dismiss_normal(bot)
    return packet.message, nil
end

local function destroy_clan(bot, npc)
    local packet = bot:request_dialog_ext(protocol.click(npc.oid), function(p)
        return p.type == "list"
    end)
    if packet == nil then
        return nil, "clan menu missing"
    end
    if select_list(bot, CAT_OPERATION, "list") == nil then
        return nil, "operation menu missing"
    end
    if select_list(bot, OP_DESTROY, "list") == nil then
        return nil, "destroy confirm missing"
    end
    packet = select_list(bot, CONFIRM_YES, "normal")
    if packet == nil then
        return nil, "destroy result missing"
    end
    dismiss_normal(bot)
    return packet.message, nil
end

local function open_vault(bot)
    local packet = bot:request_dialog(protocol.click(g_sensei.oid), function(p)
        return p.type == "pursuit"
    end)
    if packet == nil then
        return nil, "sensei menu missing"
    end
    packet = bot:request_dialog(protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_VAULT), function(p)
        return p.type == "pursuit"
    end)
    if packet == nil then
        return nil, "vault menu missing"
    end
    return packet, nil
end

local function clan_money(ctx, bot)
    ctx:sleep(MQ_WAIT_MS)
    local packet, err = open_vault(bot)
    if packet == nil then
        return nil, err
    end
    close_dialog(bot)
    local amount = packet.message and packet.message:match("(%d+)전")
    if amount == nil then
        return nil, "clan money missing msg=" .. tostring(packet.message)
    end
    return tonumber(amount), nil
end

local function deposit(bot, amount)
    local packet, err = open_vault(bot)
    if packet == nil then
        return nil, err
    end
    packet = bot:request_dialog(protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_DEPOSIT), function(p)
        return p.type == "input"
    end)
    if packet == nil then
        return nil, "deposit input missing"
    end
    packet = bot:request_dialog_ext(protocol.dialog("INPUT", 0, tostring(amount), 0, 0, "", "NEXT"), function(p)
        return p.type == "normal"
    end)
    if packet == nil then
        return nil, "deposit result missing"
    end
    dismiss_normal(bot)
    return packet.message, nil
end

local function withdraw(bot, amount)
    local packet, err = open_vault(bot)
    if packet == nil then
        return nil, err
    end
    packet = bot:request_dialog_ext(protocol.dialog("PURSUIT", 0, "", 0, 0, OPT_WITHDRAW), function(p)
        return p.type == "normal"
    end)
    if packet == nil then
        return nil, "withdraw intro missing"
    end
    packet = bot:request_dialog(protocol.dialog("NORMAL", DIALOG_NEXT, "", 0, 0, "", "NEXT"), function(p)
        return p.type == "input"
    end)
    if packet == nil then
        return nil, "withdraw input missing"
    end
    packet = bot:request_dialog_ext(protocol.dialog("INPUT", 0, tostring(amount), 0, 0, "", "NEXT"), function(p)
        return p.type == "normal"
    end)
    if packet == nil then
        return nil, "withdraw result missing"
    end
    dismiss_normal(bot)
    return packet.message, nil
end

local function wait_money(ctx, bot, expected, first_wait_ms)
    ctx:sleep(first_wait_ms)
    for attempt = 1, SETTLE_POLL_COUNT do
        if bot:money() == expected then
            return true
        end
        progress(bot, string.format("money not settled yet attempt=%d money=%d expected=%d",
            attempt, bot:money(), expected))
        ctx:sleep(SETTLE_POLL_MS)
    end
    return false
end

-- Checks both balances after a later tick, so a settled request cannot pay or refund twice.
local function assert_stable(ctx, bot, money, clan)
    ctx:sleep(SETTLE_FIRST_WAIT_MS + SETTLE_POLL_MS)
    if bot:money() ~= money then
        return false, string.format("money changed after settle money=%d expected=%d", bot:money(), money)
    end
    local current, err = clan_money(ctx, bot)
    if current == nil then
        return false, err
    end
    if current ~= clan then
        return false, string.format("clan money changed after settle clan=%d expected=%d", current, clan)
    end
    return true, nil
end

local function check_balances(ctx, bot, money, clan)
    if bot:money() ~= money then
        return false, string.format("money=%d expected=%d", bot:money(), money)
    end
    local current, err = clan_money(ctx, bot)
    if current == nil then
        return false, err
    end
    if current ~= clan then
        return false, string.format("clan money=%d expected=%d", current, clan)
    end
    return true, nil
end

test_suite {
    name      = "Clan Money Test",
    bot_count = 2,

    on_initialize = function(ctx)
        local a = ctx:bot(0)
        progress(a, "CLAN MONEY TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
        clear_faults(a)
    end,

    on_finished = function(ctx)
        clear_faults(ctx:bot(1))
    end,

    scenarios = {
        -- C0: create the clan and spawn the vault NPC
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "C0: SETUP")

            local agent = spawn_npc(a, AGENT_NPC)
            local msg, err = create_clan(a, agent)
            remove_npc(a, agent)
            if msg == nil or msg:find(MSG_CREATE_OK, 1, true) == nil then
                progress(a, "FAILED: create clan " .. tostring(msg or err))
                return false
            end

            g_sensei = spawn_npc(a, SENSEI_NPC)
            a:money(START_MONEY)
            local ok, check_err = check_balances(ctx, a, START_MONEY, 0)
            if ok == false then
                progress(a, "FAILED: " .. tostring(check_err))
                return false
            end

            progress(a, "C0 PASSED")
            return true
        end,

        -- C1: a lost deposit response is resolved by the immediate abort as applied
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "C1: DEPOSIT RESPONSE LOST, ABORT ANSWERS")

            a:chat("/HTTP장애 " .. MONEY_PATH .. " lost")
            local msg, err = deposit(a, AMOUNT)
            a:chat("/HTTP장애 " .. MONEY_PATH .. " off")
            if msg == nil or msg:find(MSG_DEPOSIT_OK, 1, true) == nil then
                progress(a, "FAILED: deposit " .. tostring(msg or err))
                return false
            end

            local ok, check_err = check_balances(ctx, a, START_MONEY - AMOUNT, AMOUNT)
            if ok == false then
                progress(a, "FAILED: " .. tostring(check_err))
                return false
            end

            progress(a, "C1 PASSED")
            return true
        end,

        -- C2: deposit applied but both the response and the abort were lost; the timer settles without a refund
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "C2: DEPOSIT APPLIED, TIMER SETTLES")

            a:chat("/HTTP장애 " .. MONEY_PATH .. " lost")
            a:chat("/HTTP장애 " .. ABORT_PATH .. " on")
            local msg, err = deposit(a, AMOUNT)
            a:chat("/HTTP장애 " .. MONEY_PATH .. " off")
            if msg == nil or msg:find(MSG_UNKNOWN, 1, true) == nil then
                a:chat("/HTTP장애 " .. ABORT_PATH .. " off")
                progress(a, "FAILED: expected unknown deposit " .. tostring(msg or err))
                return false
            end

            local money = START_MONEY - AMOUNT * 2
            local ok, check_err = check_balances(ctx, a, money, AMOUNT * 2)
            a:chat("/HTTP장애 " .. ABORT_PATH .. " off")
            if ok == false then
                progress(a, "FAILED: held deposit " .. tostring(check_err))
                return false
            end

            ok, check_err = assert_stable(ctx, a, money, AMOUNT * 2)
            if ok == false then
                progress(a, "FAILED: " .. tostring(check_err))
                return false
            end

            progress(a, "C2 PASSED")
            return true
        end,

        -- C3: a deposit that never reached internal is refunded by the timer after a tick that still fails
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "C3: DEPOSIT UNREACHABLE, TIMER REFUNDS")

            local before = START_MONEY - AMOUNT * 2
            a:chat("/HTTP장애 " .. MONEY_PATH .. " on")
            a:chat("/HTTP장애 " .. ABORT_PATH .. " on")
            local msg, err = deposit(a, AMOUNT)
            a:chat("/HTTP장애 " .. MONEY_PATH .. " off")
            if msg == nil or msg:find(MSG_UNKNOWN, 1, true) == nil then
                a:chat("/HTTP장애 " .. ABORT_PATH .. " off")
                progress(a, "FAILED: expected unknown deposit " .. tostring(msg or err))
                return false
            end

            ctx:sleep(SETTLE_FIRST_WAIT_MS + SETTLE_POLL_MS)
            if a:money() ~= before - AMOUNT then
                a:chat("/HTTP장애 " .. ABORT_PATH .. " off")
                progress(a, string.format("FAILED: hold released while abort failed money=%d", a:money()))
                return false
            end

            a:chat("/HTTP장애 " .. ABORT_PATH .. " off")
            if wait_money(ctx, a, before, SETTLE_FIRST_WAIT_MS) == false then
                progress(a, "FAILED: deposit hold not refunded")
                return false
            end
            local ok, check_err = assert_stable(ctx, a, before, AMOUNT * 2)
            if ok == false then
                progress(a, "FAILED: " .. tostring(check_err))
                return false
            end

            progress(a, "C3 PASSED")
            return true
        end,

        -- C4: requests delivered after the abort wrote its tombstone must not apply
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "C4: LATE DEPOSIT AFTER ABORT")

            local before = START_MONEY - AMOUNT * 2
            a:chat("/HTTP보류 on " .. MONEY_PATH)
            local msg, err = deposit(a, AMOUNT)
            if msg == nil or msg:find(MSG_DEPOSIT_NO, 1, true) == nil then
                a:chat("/HTTP보류 off")
                progress(a, "FAILED: expected aborted deposit " .. tostring(msg or err))
                return false
            end

            a:chat("/HTTP보류 send")
            local ok, check_err = check_balances(ctx, a, before, AMOUNT * 2)
            if ok == false then
                progress(a, "FAILED: late deposit " .. tostring(check_err))
                return false
            end

            progress(a, "C4 PASSED")
            return true
        end,

        -- C5: a withdrawal applied while every answer was lost survives relogin and pays on login
        function(ctx)
            local a = ctx:bot(0)
            local operator = ctx:bot(1)
            progress(a, "C5: WITHDRAW APPLIED, RELOGIN PAYS")

            local before = START_MONEY - AMOUNT * 2
            a:chat("/HTTP장애 " .. MONEY_PATH .. " lost")
            a:chat("/HTTP장애 " .. ABORT_PATH .. " on")
            local msg, err = withdraw(a, AMOUNT)
            if msg == nil or msg:find(MSG_UNKNOWN, 1, true) == nil then
                clear_faults(a)
                progress(a, "FAILED: expected unknown withdraw " .. tostring(msg or err))
                return false
            end

            local ok, check_err = check_balances(ctx, a, before, AMOUNT)
            if ok == false then
                clear_faults(a)
                progress(a, "FAILED: unpaid withdraw " .. tostring(check_err))
                return false
            end

            remove_npc(a, g_sensei)
            g_sensei = nil
            if a:logout() == false then
                clear_faults(operator)
                progress(a, "FAILED: logout")
                return false
            end
            clear_faults(operator)
            if a:login() == false then
                progress(a, "FAILED: relogin")
                return false
            end
            a = ctx:bot(0)
            lib.formation.arrange_in_line(ctx)
            g_sensei = spawn_npc(a, SENSEI_NPC)

            -- Checked well inside one timer period, so the payment comes from the login settle.
            ctx:sleep(MQ_WAIT_MS * 2)
            if a:money() ~= before + AMOUNT then
                progress(a, string.format("FAILED: withdraw not paid after relogin money=%d", a:money()))
                return false
            end
            ok, check_err = assert_stable(ctx, a, before + AMOUNT, AMOUNT)
            if ok == false then
                progress(a, "FAILED: " .. tostring(check_err))
                return false
            end

            progress(a, "C5 PASSED")
            return true
        end,

        -- C6: empty the vault and destroy the clan
        function(ctx)
            local a = ctx:bot(0)
            progress(a, "C6: TEARDOWN")

            local msg, err = withdraw(a, AMOUNT)
            if msg == nil or msg:find(MSG_WITHDRAW_OK, 1, true) == nil then
                progress(a, "FAILED: final withdraw " .. tostring(msg or err))
                return false
            end
            remove_npc(a, g_sensei)
            g_sensei = nil

            local agent = spawn_npc(a, AGENT_NPC)
            msg, err = destroy_clan(a, agent)
            remove_npc(a, agent)
            if msg == nil or msg:find(MSG_DESTROY_OK, 1, true) == nil then
                progress(a, "FAILED: destroy clan " .. tostring(msg or err))
                return false
            end
            a:money(0)

            progress(a, "C6 PASSED")
            return true
        end,
    },
}
