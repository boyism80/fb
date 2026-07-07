local resp     = require("integration.response")
local protocol = require("integration.protocol")

local F1_OID = 0xFFFFFFFF
local MATCH_MENU_INDEX = 2
local MATCH2_TYPE_INDEX = 1
local REGISTER_CONFIRM_TIMEOUT_MS = 10000

local MSG_NOT_GROUP_MASTER = "그룹장만 매치메이킹을 이용할 수 있습니다."
local MSG_REGISTER_START = "매치메이킹 대기를 시작했습니다"
local MSG_REGISTER_CANCEL = "매치메이킹 대기를 취소했습니다"
local MSG_MATCH_PROPOSED = "매치가 제안되었습니다"
local MSG_MATCH_CONFIRM = "매치 참여를 수락했습니다"
local MSG_MATCH_READY = "매치가 성사되었습니다"
local MSG_PROPOSAL_DIALOG = "매치를 찾았습니다"

local M = {}

local function bot_label(bot)
    return bot:name()
end

local function dialog_summary(packet)
    if packet == nil then
        return "packet=nil"
    end

    local parts = { "type=" .. tostring(packet.type) }
    if packet.message ~= nil then
        parts[#parts + 1] = "message=" .. packet.message
    end
    if packet.text ~= nil then
        parts[#parts + 1] = "text=" .. packet.text
    end
    return table.concat(parts, ", ")
end

local function mm_log(bot, flow, step, detail)
    local message = string.format(
        "mm bot=%s flow=%s step=%s",
        bot_label(bot),
        flow,
        step)
    if detail ~= nil and detail ~= "" then
        message = message .. " " .. detail
    end
    log("debug", message)
end

local function request_dialog_ext_logged(bot, flow, step, request, validator, expect)
    mm_log(bot, flow, step, "send dialog_ext expect=" .. expect)
    local packet = bot:request_dialog_ext(request, validator)
    mm_log(bot, flow, step, "recv " .. dialog_summary(packet))
    return packet
end

local function dismiss_normal_dialog(bot)
    request_dialog_ext_logged(
        bot,
        "dismiss",
        "close_normal",
        protocol.dialog("NORMAL", 0, "", 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "list" or packet.type == "normal"
        end,
        "list|normal")
end

function M.f1_open_menu(bot)
    request_dialog_ext_logged(
        bot,
        "f1",
        "open_menu",
        protocol.click(F1_OID),
        function(packet)
            return packet.type == "list"
        end,
        "list")
end

function M.f1_select_matchmaking(bot)
    M.f1_open_menu(bot)

    request_dialog_ext_logged(
        bot,
        "f1",
        "select_matchmaking",
        protocol.dialog("LIST", 0, "", MATCH_MENU_INDEX, 0, "", "NEXT"),
        function(packet)
            return packet.type == "list" or packet.type == "normal"
        end,
        "list|normal")
end

function M.f1_register_match2(bot)
    mm_log(bot, "register", "begin", "MATCH_2 via F1")

    M.f1_select_matchmaking(bot)

    request_dialog_ext_logged(
        bot,
        "register",
        "select_match2_type",
        protocol.dialog("LIST", 0, "", MATCH2_TYPE_INDEX, 0, "", "NEXT"),
        function(packet)
            return packet.type == "list"
        end,
        "list(confirm yes/no)")

    -- Successful registration closes F1 without a dialog response.
    mm_log(bot, "register", "confirm_yes", "send LIST[0], wait STATE (not dialog_ext)")
    local packet = bot:request(
        resp.message,
        protocol.dialog("LIST", 0, "", 0, 0, "", "NEXT"),
        function(p)
            if p.type == "STATE"
                and p.text ~= nil
                and p.text:find(MSG_REGISTER_START, 1, true) ~= nil then
                return true
            end
            return false
        end,
        REGISTER_CONFIRM_TIMEOUT_MS)

    if packet == nil then
        mm_log(bot, "register", "confirm_yes", "FAILED no STATE within " .. REGISTER_CONFIRM_TIMEOUT_MS .. "ms")
        return false
    end

    mm_log(bot, "register", "done", dialog_summary(packet))
    return true
end

function M.f1_fail_not_group_master(bot)
    mm_log(bot, "reject_non_leader", "begin", "non-leader opens matchmaking")

    M.f1_open_menu(bot)

    local packet = request_dialog_ext_logged(
        bot,
        "reject_non_leader",
        "select_matchmaking",
        protocol.dialog("LIST", 0, "", MATCH_MENU_INDEX, 0, "", "NEXT"),
        function(p)
            return p.type == "normal"
                and p.message ~= nil
                and p.message:find(MSG_NOT_GROUP_MASTER, 1, true) ~= nil
        end,
        "normal with not-group-master message")

    local ok = packet ~= nil
        and packet.message ~= nil
        and packet.message:find(MSG_NOT_GROUP_MASTER, 1, true) ~= nil

    if ok then
        mm_log(bot, "reject_non_leader", "done", "got expected rejection")
    else
        mm_log(bot, "reject_non_leader", "done", "FAILED unexpected " .. dialog_summary(packet))
    end

    dismiss_normal_dialog(bot)
    return ok
end

function M.f1_unregister_match2(bot)
    mm_log(bot, "unregister", "begin", "cancel enrolled match via F1")

    M.f1_select_matchmaking(bot)

    request_dialog_ext_logged(
        bot,
        "unregister",
        "confirm_cancel",
        protocol.dialog("LIST", 0, "", 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "list" or packet.type == "normal"
        end,
        "list|normal")

    mm_log(bot, "unregister", "done", "dialog flow finished")
end

function M.wait_proposal_dialog(bot, ctx)
    mm_log(bot, "proposal", "wait_dialog", "listening for proposal list dialog")

    local trace_hooked = false
    if ctx ~= nil then
        ctx:hook("message", function(_, hooked_bot, packet)
            if hooked_bot:name() ~= bot:name() then
                return
            end
            if packet.type == "STATE" and packet.text ~= nil then
                if packet.text:find("매치", 1, true) ~= nil then
                    mm_log(hooked_bot, "trace", "state", "text=" .. packet.text)
                end
            end
        end)
        trace_hooked = true
    end

    local packet = bot:request_dialog_ext(
        protocol.chat(false, "."),
        function(p)
            if p.type == "list"
                and p.message ~= nil
                and p.message:find(MSG_PROPOSAL_DIALOG, 1, true) ~= nil then
                return true
            end
            if p.type == "list" or p.type == "normal" then
                mm_log(bot, "proposal", "wait_dialog_skip", dialog_summary(p))
            end
            return false
        end)

    if trace_hooked then
        ctx:unhook("message")
    end

    if packet == nil then
        mm_log(bot, "proposal", "wait_dialog", "FAILED no proposal dialog")
    else
        mm_log(bot, "proposal", "wait_dialog", "recv " .. dialog_summary(packet))
    end
    return packet
end

function M.confirm_proposal_dialog(bot, ctx)
    local packet = M.wait_proposal_dialog(bot, ctx)
    if packet == nil or packet.message == nil then
        mm_log(bot, "proposal", "confirm", "FAILED no dialog to confirm")
        return false
    end

    -- Accepting the proposal closes the dialog without a follow-up packet.
    mm_log(bot, "proposal", "confirm_yes", "send LIST[0], wait STATE (not dialog_ext)")
    local confirm_packet = bot:request(
        resp.message,
        protocol.dialog("LIST", 0, "", 0, 0, "", "NEXT"),
        function(p)
            if p.type == "STATE"
                and p.text ~= nil
                and p.text:find(MSG_MATCH_CONFIRM, 1, true) ~= nil then
                return true
            end
            return false
        end,
        REGISTER_CONFIRM_TIMEOUT_MS)

    if confirm_packet == nil then
        mm_log(bot, "proposal", "confirm", "FAILED no STATE within " .. REGISTER_CONFIRM_TIMEOUT_MS .. "ms")
        return false
    end

    mm_log(bot, "proposal", "confirm", "done " .. dialog_summary(confirm_packet))
    return true
end

function M.wait_state_message(ctx, bot, pattern, timeout_ms)
    mm_log(bot, "state", "wait", "pattern=" .. pattern .. " timeout_ms=" .. timeout_ms)

    local found = false
    local matched_text = nil
    ctx:hook("message", function(_, hooked_bot, packet)
        if hooked_bot:name() ~= bot:name() then
            return
        end
        if packet.type == "STATE"
            and packet.text ~= nil
            and packet.text:find(pattern, 1, true) ~= nil then
            found = true
            matched_text = packet.text
        end
    end)

    local waited_ms = 0
    local interval_ms = 200
    while found == false and waited_ms < timeout_ms do
        ctx:sleep(interval_ms)
        waited_ms = waited_ms + interval_ms
    end

    ctx:unhook("message")

    if found then
        mm_log(bot, "state", "received", "text=" .. tostring(matched_text) .. " waited_ms=" .. waited_ms)
    else
        mm_log(bot, "state", "timeout", "pattern=" .. pattern .. " waited_ms=" .. waited_ms)
    end

    return found
end

function M.wait_all_state_messages(ctx, bots, pattern, timeout_ms, after_hook)
    local names = {}
    for _, bot in ipairs(bots) do
        names[#names + 1] = bot:name()
    end
    log("debug", string.format(
        "mm state wait_all pattern=%s bots=[%s] timeout_ms=%d",
        pattern,
        table.concat(names, ", "),
        timeout_ms))

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
            if received[hooked_bot:name()] == false then
                mm_log(hooked_bot, "state", "received", "text=" .. packet.text)
            end
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
            log("debug", string.format(
                "mm state wait_all done pattern=%s waited_ms=%d",
                pattern,
                waited_ms))
            return true
        end
        ctx:sleep(interval_ms)
        waited_ms = waited_ms + interval_ms
    end

    ctx:unhook("message")

    local missing = {}
    for _, bot in ipairs(bots) do
        if received[bot:name()] ~= true then
            missing[#missing + 1] = bot:name()
        end
    end
    log("debug", string.format(
        "mm state wait_all TIMEOUT pattern=%s waited_ms=%d missing=[%s]",
        pattern,
        waited_ms,
        table.concat(missing, ", ")))

    return false
end

M.MSG_REGISTER_START = MSG_REGISTER_START
M.MSG_REGISTER_CANCEL = MSG_REGISTER_CANCEL
M.MSG_MATCH_PROPOSED = MSG_MATCH_PROPOSED
M.MSG_MATCH_CONFIRM = MSG_MATCH_CONFIRM
M.MSG_MATCH_READY = MSG_MATCH_READY

return M
