local lib      = require("integration.lib")
local protocol = require("integration.protocol")

local F1_OID            = 0xFFFFFFFF
local F1_STORAGE_INDEX  = 0
local F1_GRANT_INDEX    = 3
local GRANT_TARGET_USER = 1
local GRANT_REWARD_ITEM = 0
local GRANT_REWARD_NONE = 3
local RECEIVE_YES       = 0

-- NORMAL dialog resumes with request.action (not button).
-- Must match fb::model::enum_value::DIALOG_RESULT.
local DIALOG_PREV = 0
local DIALOG_QUIT = 1
local DIALOG_NEXT = 2

local REWARD_ITEM_NAME  = "도토리"
local REWARD_ITEM_COUNT = 1
local FILL_ITEM_NAME    = "목도"
local MQ_WAIT_MS        = 2000
local HTTP_DELAY_MS     = 3000
local C2_FILL_WAIT_MS   = 1000

local MSG_GRANT_NEED_REWARD = "보상이 하나 이상 필요합니다."
local MSG_GRANT_OK          = "지급이 완료되었습니다."
local MSG_RECEIVE_OK        = "보상이 지급되었습니다."
local MSG_RECEIVE_FAIL      = "수령 조건이 맞지 않습니다"
local MSG_STATUS_AVAILABLE  = "수령 가능"
local MSG_STATUS_RECEIVED   = "수령 완료"

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
end

local function find_list_index(packet, text)
    if packet == nil or packet.list_lists == nil then
        return nil
    end
    for i, name in ipairs(packet.list_lists) do
        if name == text then
            return i - 1
        end
    end
    return nil
end

local function message_contains(packet, text)
    return packet ~= nil
        and packet.message ~= nil
        and packet.message:find(text, 1, true) ~= nil
end

local function dismiss_normal_to_list(bot)
    return bot:request_dialog_ext(
        protocol.dialog("NORMAL", DIALOG_NEXT, "", 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "list" or packet.type == "normal"
        end)
end

local function dialog_normal_next()
    return protocol.dialog("NORMAL", DIALOG_NEXT, "", 0, 0, "", "NEXT")
end

local function dialog_normal_prev()
    return protocol.dialog("NORMAL", DIALOG_PREV, "", 0, 0, "", "PREV")
end

local function f1_open_menu(bot)
    return bot:request_dialog_ext(
        protocol.click(F1_OID),
        function(packet)
            return packet.type == "list"
        end)
end

local function f1_select_list(bot, index)
    return bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(packet)
            return packet.type == "list" or packet.type == "normal"
        end)
end

local function f1_select_input(bot, index)
    return bot:request_dialog(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(packet)
            return packet.type == "input"
        end)
end

local function send_input_expect_input(bot, text)
    return bot:request_dialog(
        protocol.dialog("INPUT", 0, text, 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "input"
        end)
end

local function send_input_expect_list(bot, text)
    return bot:request_dialog_ext(
        protocol.dialog("INPUT", 0, text, 0, 0, "", "NEXT"),
        function(packet)
            return packet.type == "list" or packet.type == "normal"
        end)
end

local function grant_flow(bot_a, target_name, title, message, item_name, item_count)
    local packet = f1_open_menu(bot_a)
    if packet == nil then
        return false, "f1 open failed"
    end

    packet = f1_select_list(bot_a, F1_GRANT_INDEX)
    if packet == nil or message_contains(packet, "누구에게") == false then
        return false, "grant menu missing"
    end

    packet = f1_select_input(bot_a, GRANT_TARGET_USER)
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

    packet = send_input_expect_list(bot_a, message)
    if packet == nil then
        return false, "reward menu missing"
    end

    if item_name ~= nil then
        packet = f1_select_input(bot_a, GRANT_REWARD_ITEM)
        if packet == nil then
            return false, "item name input missing"
        end

        packet = send_input_expect_input(bot_a, item_name)
        if packet == nil then
            return false, "item count input missing"
        end

        packet = send_input_expect_list(bot_a, tostring(item_count))
        if packet == nil then
            return false, "reward menu after item missing"
        end
    end

    packet = bot_a:request_dialog_ext(
        protocol.dialog("LIST", 0, "", GRANT_REWARD_NONE, 0, "", "NEXT"),
        function(p)
            return p.type == "normal"
        end)
    if packet == nil then
        return false, "grant result dialog missing"
    end

    local result_msg = packet.message
    dismiss_normal_to_list(bot_a)
    return true, result_msg
end

local function open_storage_entry_detail(bot, title)
    local packet = f1_open_menu(bot)
    if packet == nil then
        return nil, "f1 open failed"
    end

    packet = f1_select_list(bot, F1_STORAGE_INDEX)
    if packet == nil then
        return nil, "storage list missing"
    end

    if packet.type == "normal" and message_contains(packet, "보관된 항목이 없습니다") then
        dismiss_normal_to_list(bot)
        return nil, "storage empty"
    end

    local index = find_list_index(packet, title)
    if index == nil then
        return nil, "entry not found: " .. title
    end

    packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", index, 0, "", "NEXT"),
        function(p)
            return p.type == "normal"
        end)
    if packet == nil then
        return nil, "entry detail missing"
    end
    return packet, nil
end

local function close_detail_with_prev(bot)
    bot:request_dialog_ext(
        dialog_normal_prev(),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
end

local function dump_dialog(bot, label, packet)
    if packet == nil then
        progress(bot, string.format("DIALOG[%s] nil", label))
        return
    end
    local lists = ""
    if packet.list_lists ~= nil then
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

    local packet = bot:request_dialog_ext(
        dialog_normal_next(),
        function(p)
            return p.type == "list" or p.type == "normal"
        end)
    dump_dialog(bot, "after-detail-next", packet)
    if packet == nil or packet.type ~= "list" then
        return false, "receive confirm missing"
    end
    if packet.message == nil or packet.message:find("수령하시겠습니까", 1, true) == nil then
        progress(bot, string.format("UNEXPECTED confirm dialog msg=%q", tostring(packet.message)))
        return false, "receive confirm list missing"
    end

    progress(bot, "receive_from_detail click YES")
    packet = bot:request_dialog_ext(
        protocol.dialog("LIST", 0, "", RECEIVE_YES, 0, "", "NEXT"),
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
    dismiss_normal_to_list(bot)
    return matched, result_msg
end

local function open_receive_confirm(bot, title)
    local detail, err = open_storage_entry_detail(bot, title)
    if detail == nil then
        return nil, err
    end

    local packet = bot:request_dialog_ext(
        dialog_normal_next(),
        function(p)
            return p.type == "list"
        end)
    if packet == nil then
        return nil, "receive confirm missing"
    end
    return packet, nil
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

local function prepare_one_slot_free(bot)
    bot:clear_inventory()
    bot:fill_inventory(FILL_ITEM_NAME)
    bot:drop_item(0, false)
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

        -- S2: personal grant success
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

            progress(b, "S2: WAIT MQ DELIVER")
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

        -- S3/C1: invent full -> claim not attempted, still available
        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S3/C1: RECEIVE WITH FULL INVENTORY")

            b:fill_inventory(FILL_ITEM_NAME)
            progress(b, string.format("S3/C1 after fill invent_size=%d", b:inventory_size()))
            if b:inventory_size() < 52 then
                progress(b, string.format("FAILED: invent not full after fill, size=%d", b:inventory_size()))
                return false
            end

            local detail, err = open_storage_entry_detail(b, TITLE)
            dump_dialog(b, "S3-detail", detail)
            if detail == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            -- Hook server-side notify messages during receive
            local notifies = {}
            ctx:hook("message", function(_, hooked_bot, packet)
                if hooked_bot:name() ~= b:name() then
                    return
                end
                if packet.text ~= nil then
                    table.insert(notifies, packet.text)
                    log("debug", string.format("storage_box_test S3 message: %q", packet.text))
                end
            end)

            local matched, result_msg = receive_from_detail(b, MSG_RECEIVE_FAIL)
            ctx:unhook("message")
            progress(b, string.format("S3/C1 notifies=%s", table.concat(notifies, " || ")))

            if matched == false then
                progress(b, "FAILED: expected receive fail, got=" .. tostring(result_msg))
                return false
            end

            local status_ok, status_msg = assert_entry_status(b, TITLE, MSG_STATUS_AVAILABLE)
            if status_ok == false then
                progress(b, "FAILED: expected still available, got=" .. tostring(status_msg))
                return false
            end

            progress(b, "S3/C1 PASSED")
            return true
        end,

        -- S4/C2 prep: leave one free slot, arm HTTP delay, open receive confirm
        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S4/C2 PREP: ONE SLOT FREE + HTTP DELAY + CONFIRM")

            prepare_one_slot_free(b)
            b:chat(string.format("/HTTP지연 %d", HTTP_DELAY_MS))
            ctx:sleep(200)

            local confirm, err = open_receive_confirm(b, TITLE)
            if confirm == nil then
                progress(b, "FAILED: " .. tostring(err))
                b:chat("/HTTP지연 0")
                return false
            end

            progress(b, "S4/C2 PREP DONE")
            return true
        end,

        -- S4/C2: click yes while peer task fills last slot during HTTP delay
        {
            parallel = {
                [0] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "S4/C2: CLICK RECEIVE YES")

                        local packet = b:request_dialog_ext(
                            protocol.dialog("LIST", 0, "", RECEIVE_YES, 0, "", "NEXT"),
                            function(p)
                                if p.type == "normal" and p.message ~= nil then
                                    log("debug", string.format(
                                        "storage_box_test C2 YES candidate type=%s msg=%q",
                                        tostring(p.type),
                                        tostring(p.message)))
                                end
                                return p.type == "normal" and p.message ~= nil
                            end)

                        dump_dialog(b, "C2-after-yes", packet)
                        progress(b, string.format("C2 invent_size after receive=%d", b:inventory_size()))
                        b:chat("/HTTP지연 0")

                        if packet == nil then
                            progress(b, "FAILED: receive result missing")
                            return false
                        end
                        if message_contains(packet, MSG_RECEIVE_FAIL) == false then
                            progress(b, "FAILED: expected receive fail after unclaim, got=" .. tostring(packet.message))
                            dismiss_normal_to_list(b)
                            return false
                        end

                        dismiss_normal_to_list(b)

                        local status_ok, status_msg = assert_entry_status(b, TITLE, MSG_STATUS_AVAILABLE)
                        if status_ok == false then
                            progress(b, "FAILED: expected unclaim available, got=" .. tostring(status_msg))
                            return false
                        end

                        progress(b, "S4/C2 PASSED")
                        return true
                    end,
                },
                [1] = {
                    function(ctx)
                        local b = ctx:bot(1)
                        progress(b, "S4/C2: FILL LAST SLOT DURING DELAY")
                        ctx:sleep(C2_FILL_WAIT_MS)
                        b:create_item(FILL_ITEM_NAME, 1)
                        progress(b, "S4/C2: LAST SLOT FILLED")
                        return true
                    end,
                },
            },
        },

        -- S5: happy path receive
        function(ctx)
            local b = ctx:bot(1)
            progress(b, "S5: HAPPY PATH RECEIVE")

            b:clear_inventory()
            b:chat("/HTTP지연 0")

            local detail, err = open_storage_entry_detail(b, TITLE)
            if detail == nil then
                progress(b, "FAILED: " .. tostring(err))
                return false
            end

            local matched, result_msg = receive_from_detail(b, MSG_RECEIVE_OK)
            if matched == false then
                progress(b, "FAILED: expected receive ok, got=" .. tostring(result_msg))
                return false
            end

            if b:has_item_by_name(REWARD_ITEM_NAME) == false then
                progress(b, "FAILED: reward item missing")
                return false
            end
            if b:item_count(REWARD_ITEM_NAME) < REWARD_ITEM_COUNT then
                progress(b, "FAILED: reward count mismatch")
                return false
            end

            -- storage_entries() omits received boxes, so the entry should disappear from the list.
            local packet = f1_open_menu(b)
            if packet == nil then
                progress(b, "FAILED: f1 open after receive")
                return false
            end
            packet = f1_select_list(b, F1_STORAGE_INDEX)
            dump_dialog(b, "S5-storage-after-receive", packet)
            if packet ~= nil and packet.type == "list" and find_list_index(packet, TITLE) ~= nil then
                progress(b, "FAILED: received entry still listed")
                return false
            end
            if packet ~= nil and packet.type == "normal" and message_contains(packet, "보관된 항목이 없습니다") then
                dismiss_normal_to_list(b)
            end

            progress(b, "S5 PASSED")
            return true
        end,
    },
}
