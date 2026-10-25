local lib      = require("integration.lib")
local protocol = require("integration.protocol")

local F1_OID           = 0xFFFFFFFF
local DIALOG_CLOSE_OID = 0xFFFFFFFD
local OPT_STORAGE      = "통합보관함"
local OPT_GRANT        = "아이템 지급"
local OPT_TARGET_USER  = "특정 유저"
local OPT_REWARD_ITEM  = "아이템"
local OPT_REWARD_NONE  = "없음"
local OPT_YES          = "예"

-- NORMAL dialog resumes with request.action (not button).
-- Must match fb::model::enum_value::DIALOG_RESULT.
local DIALOG_PREV = 0
local DIALOG_NEXT = 2

local REWARD_ITEM_NAME  = "도토리"
local REWARD_ITEM_COUNT = 1
local FILL_ITEM_NAME    = "목도"
local MQ_WAIT_MS        = 2000
local SYSTEM_PATH       = "/storage/system"

local MSG_GRANT_NEED_REWARD = "보상이 하나 이상 필요합니다."
local MSG_GRANT_OK          = "지급이 완료되었습니다."
local MSG_GRANT_RETRY       = "같은 지급을 다시 보낼까요"
local MSG_RECEIVE_OK        = "보상이 지급되었습니다."
local MSG_RECEIVE_FULL      = "소지품이 가득 차서 보상을 받을 수 없습니다"
local MSG_STATUS_AVAILABLE  = "수령 가능"

local TITLE   = nil
local MESSAGE = nil

local function progress(bot, message)
    local level = "debug"
    if message:find("FAILED", 1, true) ~= nil then
        level = "fatal"
    end
    log(level, string.format("storage_box_test bot=%s %s", bot:name(), message))
    bot:chat("=== " .. message .. " ===")
end

local function cleanup_bot(bot)
    bot:chat("/아이템초기화")
    bot:chat("/아이템삭제")
    bot:money(0)
    bot:chat("/HTTP지연 0")
    bot:chat("/HTTP장애 " .. SYSTEM_PATH .. " off")
    bot:chat("/HTTP보류 off")
end

local function find_menu_option(packet, text)
    if packet == nil then
        return nil
    end
    local menus = packet.menu_menus or packet.list_lists
    if menus == nil then
        return nil
    end
    for _, name in ipairs(menus) do
        if name == text then
            return name
        end
    end
    return nil
end

local function count_menu_option(packet, text)
    local count = 0
    local menus = packet ~= nil and (packet.menu_menus or packet.list_lists) or nil
    for _, name in ipairs(menus or {}) do
        if name == text then
            count = count + 1
        end
    end
    return count
end

local function message_contains(packet, text)
    return packet ~= nil
        and packet.message ~= nil
        and packet.message:find(text, 1, true) ~= nil
end

local function dialog_normal_next()
    return protocol.dialog("NORMAL", DIALOG_NEXT, "", 0, 0, "", "NEXT")
end

local function dialog_normal_prev()
    return protocol.dialog("NORMAL", DIALOG_PREV, "", 0, 0, "", "PREV")
end

local function pursuit_select(option)
    return protocol.dialog("PURSUIT", 0, "", 0, 0, option)
end

-- TOP releases any waiting 0x2F/0x30 dialog without depending on the next script goto.
local function close_dialog(bot)
    bot:send(protocol.click(DIALOG_CLOSE_OID))
end

local function f1_open_menu(bot)
    return bot:request_dialog(
        protocol.click(F1_OID),
        function(packet)
            return packet.type == "pursuit"
        end)
end

local function f1_select_pursuit(bot, option, expect_type)
    expect_type = expect_type or "pursuit"
    local use_ext = (expect_type == "list" or expect_type == "normal" or expect_type == "input_ext")
    local fn = use_ext and bot.request_dialog_ext or bot.request_dialog
    return fn(bot,
        pursuit_select(option),
        function(packet)
            return packet.type == expect_type
        end)
end

local function send_input_expect_input(bot, text)
    return bot:request_dialog(
        protocol.dialog("INPUT", 0, text, 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "input"
        end)
end

local function send_input_expect_pursuit(bot, text)
    return bot:request_dialog(
        protocol.dialog("INPUT", 0, text, 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "pursuit"
        end)
end

local function grant_flow(bot_a, target_name, title, message, item_name, item_count, expect_retry)
    local packet = f1_open_menu(bot_a)
    if packet == nil then
        return false, "f1 open failed"
    end

    packet = f1_select_pursuit(bot_a, OPT_GRANT, "pursuit")
    if packet == nil or message_contains(packet, "누구에게") == false then
        return false, "grant menu missing"
    end

    packet = f1_select_pursuit(bot_a, OPT_TARGET_USER, "input")
    if packet == nil then
        return false, "name input missing"
    end

    packet = send_input_expect_input(bot_a, target_name)
    if packet == nil then
        return false, "title input missing"
    end

    packet = send_input_expect_input(bot_a, title)
    if packet == nil then
        return false, "message input missing"
    end

    packet = send_input_expect_pursuit(bot_a, message)
    if packet == nil then
        return false, "reward menu missing"
    end

    if item_name ~= nil then
        packet = f1_select_pursuit(bot_a, OPT_REWARD_ITEM, "input")
        if packet == nil then
            return false, "item name input missing"
        end

        packet = send_input_expect_input(bot_a, item_name)
        if packet == nil then
            return false, "item count input missing"
        end

        packet = send_input_expect_pursuit(bot_a, tostring(item_count))
        if packet == nil then
            return false, "reward menu after item missing"
        end
    end

    if expect_retry then
        packet = f1_select_pursuit(bot_a, OPT_REWARD_NONE, "pursuit")
        if packet == nil then
            return false, "grant retry dialog missing"
        end
        return true, packet.message
    end

    packet = bot_a:request_dialog_ext(
        pursuit_select(OPT_REWARD_NONE),
        function(p)
            return p.type == "normal"
        end)
    if packet == nil then
        return false, "grant result dialog missing"
    end

    local result_msg = packet.message
    -- grant_storage returns to F1; TOP is enough because later steps reopen F1.
    close_dialog(bot_a)
    return true, result_msg
end

local function open_storage_list(bot)
    local packet = f1_open_menu(bot)
    if packet == nil then
        return nil, "f1 open failed"
    end

    packet = f1_select_pursuit(bot, OPT_STORAGE, "pursuit")
    if packet == nil then
        return nil, "storage list missing"
    end
    return packet, nil
end

local function open_storage_entry_detail(bot, title)
    local packet, err = open_storage_list(bot)
    if packet == nil then
        return nil, err
    end

    local option = find_menu_option(packet, title)
    if option == nil then
        close_dialog(bot)
        return nil, "entry not found: " .. title
    end

    packet = bot:request_dialog_ext(
        pursuit_select(option),
        function(p)
            return p.type == "normal"
        end)
    if packet == nil then
        return nil, "entry detail missing"
    end
    return packet, nil
end

local function close_detail_with_prev(bot)
    -- PREV returns to STORAGE_LIST; entry still exists so response is pursuit.
    bot:request_dialog(
        dialog_normal_prev(),
        function(p)
            return p.type == "pursuit"
        end)
    close_dialog(bot)
end

local function dump_dialog(bot, label, packet)
    if packet == nil then
        progress(bot, string.format("DIALOG[%s] nil", label))
        return
    end
    local lists = ""
    if packet.menu_menus ~= nil then
        lists = table.concat(packet.menu_menus, "|")
    elseif packet.list_lists ~= nil then
        lists = table.concat(packet.list_lists, "|")
    end
    progress(bot, string.format(
        "DIALOG[%s] type=%s msg=%q lists=%q",
        label,
        tostring(packet.type),
        tostring(packet.message),
        lists))
end

local function receive_from_detail(bot, expect_text)
    progress(bot, string.format("receive_from_detail invent_size=%s expect=%q",
        tostring(bot:inventory_size()),
        expect_text))

    local packet = bot:request_dialog(
        dialog_normal_next(),
        function(p)
            return p.type == "pursuit"
        end)
    dump_dialog(bot, "after-detail-next", packet)
    if packet == nil or packet.type ~= "pursuit" then
        return false, "receive confirm missing"
    end
    if packet.message == nil or packet.message:find("수령하시겠습니까", 1, true) == nil then
        progress(bot, string.format("UNEXPECTED confirm dialog msg=%q", tostring(packet.message)))
        return false, "receive confirm list missing"
    end

    progress(bot, "receive_from_detail click YES")
    packet = bot:request_dialog_ext(
        pursuit_select(OPT_YES),
        function(p)
            if p.type == "normal" and p.message ~= nil then
                log("debug", string.format(
                    "storage_box_test receive YES candidate type=%s msg=%q",
                    tostring(p.type),
                    tostring(p.message)))
            end
            return p.type == "normal" and p.message ~= nil
        end)
    dump_dialog(bot, "after-receive-yes", packet)
    if packet == nil then
        return false, "receive result missing"
    end

    local matched = message_contains(packet, expect_text)
    local result_msg = packet.message
    progress(bot, string.format("receive_from_detail matched=%s msg=%q", tostring(matched), tostring(result_msg)))
    -- After result, storage.lua goto STORAGE_LIST (empty→normal / remaining→pursuit).
    -- TOP closes without depending on that branch.
    close_dialog(bot)
    return matched, result_msg
end

local function assert_entry_status(bot, title, status_text)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail == nil then
        return false, err
    end
    local ok = message_contains(detail, status_text)
    local msg = detail.message
    close_detail_with_prev(bot)
    return ok, msg
end

-- The anchor box keeps the storage list non-empty, so a missing title means the box is no longer listed.
local function assert_entry_gone(bot, title)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail ~= nil then
        close_detail_with_prev(bot)
        return false, "entry still listed: " .. title
    end
    if err ~= "entry not found: " .. title then
        return false, err
    end
    return true, nil
end

local function receive_title(bot, title)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail == nil then
        return false, err
    end
    return receive_from_detail(bot, MSG_RECEIVE_OK)
end

test_suite {
    name      = "Storage Box Test",
    bot_count = 2,

    on_initialize = function(ctx)
        local a = ctx:bot(0)
        local b = ctx:bot(1)
        TITLE   = string.format("SB_%s_%s", a:name(), b:name())
        MESSAGE = "SB_TEST_MSG"
        progress(a, "STORAGE BOX TEST INITIALIZED")
        lib.formation.arrange_in_line(ctx)
        cleanup_bot(a)
        cleanup_bot(b)
    end,

    on_finished = function(ctx)
        cleanup_bot(ctx:bot(0))
        cleanup_bot(ctx:bot(1))
    end,

    scenarios = {
        -- S1: grant without reward must fail
        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "S1: GRANT WITHOUT REWARD")

            local ok, msg = grant_flow(a, b:name(), TITLE .. "_NOREWARD", MESSAGE, nil, nil)
            if ok == false then
                progress(a, "FAILED: " .. tostring(msg))
                return false
            end
            if msg == nil or msg:find(MSG_GRANT_NEED_REWARD, 1, true) == nil then
                progress(a, "FAILED: expected need-reward message, got=" .. tostring(msg))
                return false
            end

            progress(a, "S1 PASSED")
            return true
        end,

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(a, "S2: PERSONAL GRANT")

            local ok, msg = grant_flow(a, b:name(), TITLE, MESSAGE, REWARD_ITEM_NAME, REWARD_ITEM_COUNT)
            if ok == false then
                progress(a, "FAILED: " .. tostring(msg))
                return false
            end
            if msg == nil or msg:find(MSG_GRANT_OK, 1, true) == nil then
                progress(a, "FAILED: expected grant ok, got=" .. tostring(msg))
                return false
            end

            progress(b, "S2: WAIT MQ NOTICE")
            ctx:sleep(MQ_WAIT_MS)

            local detail, err = open_storage_entry_detail(b, TITLE)
            if detail == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end
            if message_contains(detail, MSG_STATUS_AVAILABLE) == false then
                progress(b, "FAILED: entry not available, msg=" .. tostring(detail.message))
                close_detail_with_prev(b)
                return false
            end
            close_detail_with_prev(b)

            progress(a, "S2 PASSED")
            return true
        end,

        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S3: RECEIVE WITH FULL INVENTORY")

            b:fill_inventory(FILL_ITEM_NAME)
            if b:inventory_size() < 52 then
                progress(b, string.format("FAILED: invent not full after fill, size=%d", b:inventory_size()))
                return false
            end

            local detail, err = open_storage_entry_detail(b, TITLE)
            if detail == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            local matched, result_msg = receive_from_detail(b, MSG_RECEIVE_FULL)
            if matched == false then
                progress(b, "FAILED: expected receive fail, got=" .. tostring(result_msg))
                return false
            end

            local status_ok, status_msg = assert_entry_status(b, TITLE, MSG_STATUS_AVAILABLE)
            if status_ok == false then
                progress(b, "FAILED: expected still available, got=" .. tostring(status_msg))
                return false
            end

            progress(b, "S3 PASSED")
            return true
        end,

        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S4: HAPPY PATH RECEIVE")

            b:clear_inventory()
            local matched, result_msg = receive_title(b, TITLE)
            if matched == false then
                progress(b, "FAILED: expected receive ok, got=" .. tostring(result_msg))
                return false
            end
            if b:item_count(REWARD_ITEM_NAME) ~= REWARD_ITEM_COUNT then
                progress(b, string.format("FAILED: reward count=%d", b:item_count(REWARD_ITEM_NAME)))
                return false
            end

            progress(b, "S4 PASSED")
            return true
        end,

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            progress(b, "A0: ANCHOR BOX")

            local ok, msg = grant_flow(a, b:name(), TITLE .. "_ANCHOR", MESSAGE, REWARD_ITEM_NAME, REWARD_ITEM_COUNT)
            if ok == false or msg == nil or msg:find(MSG_GRANT_OK, 1, true) == nil then
                progress(a, "FAILED: anchor grant " .. tostring(msg))
                return false
            end
            ctx:sleep(MQ_WAIT_MS)

            local ok_gone, err = assert_entry_gone(b, TITLE)
            if ok_gone == false then
                progress(b, "FAILED: received box listed again: " .. tostring(err))
                return false
            end

            progress(b, "A0 PASSED")
            return true
        end,

        function(ctx)
            local a = ctx:bot(0)
            local b = ctx:bot(1)
            local title = TITLE .. "_G1"
            progress(a, "G1: GRANT RESPONSE LOST, RETRY WITH SAME REF")

            a:chat("/HTTP장애 " .. SYSTEM_PATH .. " lost")
            ctx:sleep(300)
            local ok, msg = grant_flow(a, b:name(), title, MESSAGE, REWARD_ITEM_NAME, REWARD_ITEM_COUNT, true)
            a:chat("/HTTP장애 " .. SYSTEM_PATH .. " off")
            if ok == false or msg == nil or msg:find(MSG_GRANT_RETRY, 1, true) == nil then
                close_dialog(a)
                progress(a, "FAILED: expected retry prompt, got=" .. tostring(msg))
                return false
            end
            ctx:sleep(300)

            local packet = a:request_dialog_ext(
                pursuit_select(OPT_YES),
                function(p)
                    return p.type == "normal"
                end)
            close_dialog(a)
            if message_contains(packet, MSG_GRANT_OK) == false then
                progress(a, "FAILED: retry result " .. tostring(packet and packet.message))
                return false
            end

            ctx:sleep(MQ_WAIT_MS)
            local list, err = open_storage_list(b)
            if list == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end
            local count = count_menu_option(list, title)
            close_dialog(b)
            if count ~= 1 then
                progress(b, string.format("FAILED: box count=%d expected=1", count))
                return false
            end

            progress(a, "G1 PASSED")
            return true
        end,

        function(ctx)
            local b = ctx:bot(1)
            local title = TITLE .. "_G1"
            progress(b, "G2: RECEIVE THEN RELOGIN")

            b:clear_inventory()
            local matched, result_msg = receive_title(b, title)
            if matched == false then
                progress(b, "FAILED: receive " .. tostring(result_msg))
                return false
            end

            if b:logout() == false then
                progress(b, "FAILED: logout")
                return false
            end
            if b:login() == false then
                progress(b, "FAILED: relogin")
                return false
            end
            b = ctx:bot(1)
            ctx:sleep(MQ_WAIT_MS)
            lib.formation.arrange_in_line(ctx)

            if b:item_count(REWARD_ITEM_NAME) ~= REWARD_ITEM_COUNT then
                progress(b, string.format("FAILED: reward count after relogin=%d", b:item_count(REWARD_ITEM_NAME)))
                return false
            end
            local gone, err = assert_entry_gone(b, title)
            if gone == false then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end
            local anchor = assert_entry_status(b, TITLE .. "_ANCHOR", MSG_STATUS_AVAILABLE)
            if anchor == false then
                progress(b, "FAILED: unreceived anchor lost after relogin")
                return false
            end

            progress(b, "G2 PASSED")
            return true
        end,

        function(ctx)
            local b = ctx:bot(1)
            progress(b, "F1: RECEIVE ANCHOR")

            b:clear_inventory()
            local matched, result_msg = receive_title(b, TITLE .. "_ANCHOR")
            if matched == false then
                progress(b, "FAILED: anchor receive " .. tostring(result_msg))
                return false
            end

            progress(b, "F1 PASSED")
            return true
        end,
    },
}
